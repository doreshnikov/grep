#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "file_counter.h"
#include "file_indexer.h"
#include "string_finder.h"
#include "watch_index_remover.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QThread>
#include <QtWidgets>

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), _dirs(), _unindexed_dirs(),
                                          _worker_threads(),
                                          _watcher(nullptr), _watcher_thread(nullptr),
                                          _unindexed_amount(0), _file_indexes() {
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->plainTextEdit_Error->setReadOnly(true);
    ui->plainTextEdit_Error->setStyleSheet("QPlainTextEdit {"
                                           "    color: red;"
                                           "}");
    ui->progressBar->reset();

    qRegisterMetaType<file_index>("file_index");
    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    QCommonStyle style;
    ui->action_About->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeWidget->setUniformRowHeights(true);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked,
            this, &MainWindow::removeDirectory);

    connect(ui->action_Select_Directory, &QAction::triggered,
            this, &MainWindow::selectDirectory);
    connect(ui->action_Exit, &QAction::triggered,
            this, &MainWindow::close);
    connect(ui->action_About, &QAction::triggered,
            this, &MainWindow::showAboutDialog);

    connect(ui->buttonIndex, &QPushButton::clicked,
            this, &MainWindow::startIndexing);
    connect(ui->buttonSearch, &QPushButton::clicked,
            this, &MainWindow::startSearching);

    connect(ui->lineEdit, &QLineEdit::returnPressed,
            ui->buttonSearch, &QPushButton::click);

    _watcher = std::make_unique<watcher>();
    _watcher_thread = new QThread();

    _watcher->moveToThread(_watcher_thread);
    _watcher_thread->start();

    connect(_watcher.get(), &watcher::onFileChanged,
            this, &MainWindow::onFileChanged);
    connect(_watcher.get(), &watcher::onDirectoryChanged,
            this, &MainWindow::onDirectoryChanged);

    reset_index_button();
}

void MainWindow::interrupt_workers() {
    for (auto &workerThread : _worker_threads) {
        if (workerThread != nullptr) {
            if (workerThread->isRunning()) {
                workerThread->requestInterruption();
                workerThread->wait();
            }
        }
        delete workerThread;
        workerThread = nullptr;
    }
    _worker_threads.clear();
}

QThread *MainWindow::request_new_thread() {
    QThread *thread = new QThread();
    _worker_threads.append(thread);
    return thread;
}

void MainWindow::selectDirectory() {
    QString _dir = QFileDialog::getExistingDirectoryUrl(this, "Please select a directory for indexing", QString(), QFileDialog::ShowDirsOnly).path();
    if (_dir == "" || _dir.isNull() || _dirs.contains(_dir)) {
        return;
    }
    _unindexed_dirs.insert(_dir, 0);

    ui->buttonIndex->setDisabled(true);
    ui->buttonIndex->repaint();
    ui->buttonSearch->setDisabled(true);
    ui->buttonSearch->repaint();

    ui->action_Select_Directory->setDisabled(true);
    ui->statusBar->showMessage(QString("Counting files..."));

    interrupt_workers();
    QThread *workerThread = request_new_thread();

    file_counter *counter = new file_counter(_dir);
    counter->moveToThread(workerThread);
    connect(workerThread, &QThread::started,
            counter, &file_counter::startCounting);
    connect(counter, &file_counter::onComplete,
            this, &MainWindow::onCountComplete);
    connect(workerThread, &QThread::finished,
            counter, &QObject::deleteLater);

    disconnect(ui->lineEdit, &QLineEdit::returnPressed,
               ui->buttonSearch, &QPushButton::click);

    workerThread->start();

    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
}

void MainWindow::showAboutDialog() {
    QMessageBox::about(this, "grep", "Files substring searcher with indexing and modification subscription");
}

void MainWindow::update_status_bar() {
    _file_indexes_mutex.lock();

    if (_unindexed_amount == 0) {
        ui->statusBar->showMessage(QString("Indexed files: %1").arg(_file_indexes.size()));
    } else if (_file_indexes.size() == 0) {
        ui->statusBar->showMessage(QString("Unindexed files: %1").arg(_unindexed_amount));
    } else {
        ui->statusBar->showMessage(QString("Indexed files: %1, unindexed: %2").arg(_file_indexes.size()).arg(_unindexed_amount));
    }

    _file_indexes_mutex.unlock();
}

void MainWindow::reset_index_button() {
    if (_unindexed_dirs.empty()) {
        ui->buttonIndex->setDisabled(true);
    } else {
        ui->buttonIndex->setDisabled(false);
    }
    ui->buttonIndex->repaint();
}

void MainWindow::onCountComplete(QString const &dir, int amount, qint64 size) {
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    item->setText(dir);
    item->setTextColor(QColor(255, 0, 0));

    _dirs.insert(dir, item);

    _unindexed_amount += amount;
    _unindexed_dirs[dir] = amount;

    update_status_bar();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(_unindexed_amount == 0 ? 1 : _unindexed_amount);

    interrupt_workers();

    ui->buttonIndex->setDisabled(false);
    ui->buttonIndex->repaint();
    ui->buttonSearch->setDisabled(false);
    ui->buttonSearch->repaint();
    connect(ui->lineEdit, &QLineEdit::returnPressed,
            ui->buttonSearch, &QPushButton::click);

    ui->action_Select_Directory->setDisabled(false);
}

void MainWindow::startIndexing() {
    ui->progressBar->setValue(0);

    ui->buttonIndex->setText("Abort indexing");
    ui->buttonIndex->repaint();
    disconnect(ui->buttonIndex, &QPushButton::clicked,
               this, &MainWindow::startIndexing);
    connect(ui->buttonIndex, &QPushButton::clicked,
            this, &MainWindow::stopIndexing);
    disconnect(ui->lineEdit, &QLineEdit::returnPressed,
               ui->buttonSearch, &QPushButton::click);

    _file_indexes_mutex.lock();

    ui->buttonSearch->setDisabled(true);
    ui->buttonSearch->repaint();

    for (auto const &dir : _unindexed_dirs.keys()) {
        QThread *worker_thread = request_new_thread();
        file_indexer *indexer = new file_indexer(dir);
        indexer->moveToThread(worker_thread);

        connect(worker_thread, &QThread::started,
                indexer, &file_indexer::startIndexing);
        connect(indexer, &file_indexer::onComplete,
                this, &MainWindow::onIndexComplete);
        connect(indexer, &file_indexer::onFileIndexed,
                this, &MainWindow::receiveIndexedFile);

        connect(worker_thread, &QThread::finished,
                indexer, &QObject::deleteLater);
        connect(indexer, &file_indexer::onError,
                this, &MainWindow::receiveError);

        worker_thread->start();
    }
}

void MainWindow::reindex(QString const &file_name) {
    file_indexer *indexer = new file_indexer(file_name);
    QThread *workerThread = new QThread();
    indexer->moveToThread(workerThread);

    connect(workerThread, &QThread::started,
            indexer, &file_indexer::startIndexing);
    connect(indexer, &file_indexer::onFileIndexed,
            this, &MainWindow::receiveReindexedFile);
    connect(indexer, &file_indexer::onError,
            this, &MainWindow::receiveError);
    connect(indexer, &file_indexer::onComplete,
            workerThread, &QThread::quit);
    connect(workerThread, &QThread::finished,
            indexer, &QObject::deleteLater);

    workerThread->start();
}

void MainWindow::stopIndexing() {
    _file_indexes_mutex.unlock();
    interrupt_workers();

    ui->buttonIndex->setText("Start indexing");
    ui->buttonIndex->repaint();
    connect(ui->buttonIndex, &QPushButton::clicked,
               this, &MainWindow::startIndexing);
    disconnect(ui->buttonIndex, &QPushButton::clicked,
            this, &MainWindow::stopIndexing);

    update_status_bar();
    ui->progressBar->setValue(0);

    ui->buttonSearch->setDisabled(false);
    ui->buttonSearch->repaint();
    connect(ui->lineEdit, &QLineEdit::returnPressed,
            ui->buttonSearch, &QPushButton::click);
}

void MainWindow::onIndexComplete(QString const &dir) {
    _watcher->add_path(dir);
    if (_dirs[dir] != nullptr) {
        _dirs[dir]->setTextColor(QColor(0, 50, 0));
        _unindexed_amount -= _unindexed_dirs[dir];
    }
    _unindexed_dirs.remove(dir);
    if (_unindexed_dirs.empty()) {
        ui->buttonIndex->setDisabled(true);
        stopIndexing();
        ui->progressBar->setValue(ui->progressBar->maximum());
    }
}

void MainWindow::startSearching() {
    ui->buttonIndex->setDisabled(true);
    ui->buttonIndex->repaint();

    ui->treeWidget->clear();

    ui->buttonSearch->setText("Abort searching");
    ui->buttonSearch->repaint();

    disconnect(ui->buttonSearch, &QPushButton::clicked,
               this, &MainWindow::startSearching);
    connect(ui->buttonSearch, &QPushButton::clicked,
            this, &MainWindow::stopSearching);

    QString substring = ui->lineEdit->text();
    QThread *workerThread = request_new_thread();
    _file_indexes_mutex.lock();

    string_finder *finder = new string_finder(_file_indexes, substring);
    finder->moveToThread(workerThread);

    connect(workerThread, &QThread::started,
            finder, &string_finder::startScanning);
    connect(finder, &string_finder::onInstancesFound,
            this, &MainWindow::receiveInstances);
    connect(finder, &string_finder::onError,
            this, &MainWindow::receiveError);
    connect(finder, &string_finder::onComplete,
            this, &MainWindow::onSearchComplete);
    connect(workerThread, &QThread::finished,
            finder, &QObject::deleteLater);

    workerThread->start();

    ui->statusBar->showMessage(QString("Searching ..."));
}

void MainWindow::stopSearching() {
    interrupt_workers();
    _file_indexes_mutex.unlock();

    ui->buttonSearch->setText("Start searching");
    ui->buttonSearch->repaint();
    connect(ui->buttonSearch, &QPushButton::clicked,
            this, &MainWindow::startSearching);
    disconnect(ui->buttonSearch, &QPushButton::clicked,
               this, &MainWindow::stopSearching);

    update_status_bar();
    reset_index_button();
}

void MainWindow::onSearchComplete() {
    stopSearching();
}

void MainWindow::onFileChanged(const QString &file_name) {
    _file_indexes_mutex.lock();

    if (_file_indexes.contains(file_name)) {
        _file_indexes.remove(file_name);
    }

    _file_indexes_mutex.unlock();
    if (QFileInfo(file_name).exists()) {
        reindex(file_name);
    }
    ui->plainTextEdit_Error->appendPlainText(QString("file %1 changed").arg(file_name));
}

void MainWindow::onDirectoryChanged(const QString &dir_name) {
    if (!QFileInfo(dir_name).exists()) {
        if (_dirs.contains(dir_name)) {
            delete _dirs[dir_name];
            _dirs.remove(dir_name);
        }
    }
    ui->plainTextEdit_Error->appendPlainText(QString("dir %1 changed").arg(dir_name));
}

void MainWindow::receiveIndexedFile(const file_index &index) {
    _watcher->add_path(index.get_file_path());
    ui->progressBar->setValue(ui->progressBar->value() + 1);
    if (!index.empty()) {
        _file_indexes[index.get_file_path()] = index;
    }
}

void MainWindow::receiveReindexedFile(const file_index &index) {
    _file_indexes_mutex.lock();

    if (!index.empty()) {
        _file_indexes[index.get_file_path()] = index;
    }

    _file_indexes_mutex.unlock();
}

void MainWindow::receiveInstances(const QString &file_name, const QVector<QString> &where) {
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, file_name);
    item->setText(1, QString::number(where.size()));

    for (QString const &line : where) {
        QTreeWidgetItem *child = new QTreeWidgetItem(item);
        child->setText(0, line);
    }
}

void MainWindow::receiveError(QString const &error) {
    ui->plainTextEdit_Error->appendHtml(error);
}

void MainWindow::removeDirectory(QListWidgetItem *item) {
    if (!_file_indexes_mutex.try_lock()) {
        return;
    }
    item->setTextColor(QColor(255, 0, 0));
    QString dir = item->text();
    _unindexed_dirs.insert(dir, 0);

    ui->progressBar->setValue(0);

    ui->buttonIndex->setText("Abort removal");
    ui->buttonIndex->repaint();
    disconnect(ui->buttonIndex, &QPushButton::clicked,
               this, &MainWindow::startIndexing);
    connect(ui->buttonIndex, &QPushButton::clicked,
            this, &MainWindow::stopRemoving);
    disconnect(ui->lineEdit, &QLineEdit::returnPressed,
               ui->buttonSearch, &QPushButton::click);

    ui->buttonSearch->setDisabled(true);
    ui->buttonSearch->repaint();

    QThread *worker_thread = request_new_thread();
    watch_index_remover *remover = new watch_index_remover(dir);
    remover->moveToThread(worker_thread);

    connect(worker_thread, &QThread::started,
            remover, &watch_index_remover::startRemoving);
    connect(remover, &watch_index_remover::onComplete,
            this, &MainWindow::onRemovingComplete);
    connect(remover, &watch_index_remover::onFileMet,
            this, &MainWindow::onIndexedFileRemoved);
    connect(remover, &watch_index_remover::onDirectoryMet,
            this, &MainWindow::onIndexedDirectoryRemoved);

    connect(worker_thread, &QThread::finished,
            remover, &QObject::deleteLater);
    connect(worker_thread, &QThread::finished,
            this, &MainWindow::stopRemoving);

    connect(remover, &watch_index_remover::onError,
            this, &MainWindow::receiveError);

    worker_thread->start();

    ui->statusBar->showMessage(QString("Removing directory from indexes..."));
}

void MainWindow::stopRemoving() {
    interrupt_workers();

    ui->buttonIndex->setText("Start indexing");
    ui->buttonIndex->repaint();
    connect(ui->buttonIndex, &QPushButton::clicked,
               this, &MainWindow::startIndexing);
    disconnect(ui->buttonIndex, &QPushButton::clicked,
            this, &MainWindow::stopRemoving);
    ui->progressBar->setValue(0);

    ui->buttonSearch->setDisabled(false);
    ui->buttonSearch->repaint();
    connect(ui->lineEdit, &QLineEdit::returnPressed,
            ui->buttonSearch, &QPushButton::click);

    _file_indexes_mutex.unlock();
    update_status_bar();
    reset_index_button();
}

void MainWindow::onRemovingComplete(const QString &dir) {
    ui->plainTextEdit_Error->appendPlainText(QString("%1 removed").arg(dir));
    stopRemoving();
    delete _dirs[dir];
    _dirs.remove(dir);
    if (_unindexed_dirs.contains(dir)) {
        _unindexed_amount -= _unindexed_dirs[dir];
        _unindexed_dirs.remove(dir);
    }

    ui->progressBar->setValue(ui->progressBar->maximum());
}

void MainWindow::onIndexedFileRemoved(const QString &file_name) {
    _watcher->remove_path(file_name);
    _file_indexes.remove(file_name);
}

void MainWindow::onIndexedDirectoryRemoved(const QString &dir_name) {
    _watcher->remove_path(dir_name);
    if (_dirs.contains(dir_name)) {
        delete _dirs[dir_name];
        _dirs.remove(dir_name);
    }
    if (_unindexed_dirs.contains(dir_name)) {
        _unindexed_amount -= _unindexed_dirs[dir_name];
        _unindexed_dirs.remove(dir_name);
    }
}

MainWindow::~MainWindow() {
    interrupt_workers();
    _watcher_thread->quit();
    _watcher_thread->wait();
    delete _watcher_thread;
}
