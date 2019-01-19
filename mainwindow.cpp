#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "file_counter.h"
#include "file_indexer.h"
#include "string_finder.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QThread>
#include <QtWidgets>

#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), _dirs(), _unindexed_dirs(),
                                          _workerThreads(),
                                          _unindexed_amount(0), _file_indexes() {
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->plainTextEdit_Error->setReadOnly(true);
    ui->plainTextEdit_Error->setStyleSheet("QPlainTextEdit {"
                                           "    color: red;"
                                           "}");
    ui->progressBar->reset();

    qRegisterMetaType<file_index>("file_index");
    qRegisterMetaType<QVector<QPair<quint64, QString>>>("QVector<QPair<quint64, QString>>");
    QCommonStyle style;
    ui->action_About->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeWidget->setUniformRowHeights(true);

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

    resetButtonIndex();
}

void MainWindow::interruptWorkers() {
    for (auto &workerThread : _workerThreads) {
        if (workerThread != nullptr) {
            if (workerThread->isRunning()) {
                workerThread->requestInterruption();
                workerThread->wait();
            }
        }
        delete workerThread;
        workerThread = nullptr;
    }
    _workerThreads.clear();
}

QThread *MainWindow::requestNewThread() {
    QThread *thread = new QThread();
    _workerThreads.append(thread);
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

    interruptWorkers();
    QThread *workerThread = requestNewThread();

    file_counter *counter = new file_counter(_dir);
    counter->moveToThread(workerThread);
    connect(workerThread, &QThread::started,
            counter, &file_counter::startCounting);
    connect(counter, &file_counter::onComplete,
            this, &MainWindow::onCountComplete);
    connect(workerThread, &QThread::finished,
            counter, &QObject::deleteLater);

    workerThread->start();

    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
}

void MainWindow::showAboutDialog() {
    QMessageBox::about(this, "fdupes", "Files duplicates finder utility");
}

void MainWindow::updateStatusBar() {
    if (_unindexed_amount == 0) {
        ui->statusBar->showMessage(QString("Indexed files: %1").arg(_file_indexes.size()));
    } else if (_file_indexes.size() == 0) {
        ui->statusBar->showMessage(QString("Unindexed files: %1").arg(_unindexed_amount));
    } else {
        ui->statusBar->showMessage(QString("Indexed files: %1, unindexed: %2").arg(_file_indexes.size()).arg(_unindexed_amount));
    }
}

void MainWindow::resetButtonIndex() {
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

    updateStatusBar();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(_unindexed_amount == 0 ? 1 : _unindexed_amount);

    interruptWorkers();

    ui->buttonIndex->setDisabled(false);
    ui->buttonIndex->repaint();
    ui->buttonSearch->setDisabled(false);
    ui->buttonSearch->repaint();

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

    ui->buttonSearch->setDisabled(true);
    ui->buttonSearch->repaint();

    for (auto const &dir : _dirs.keys()) {
        QThread *workerThread = requestNewThread();
        file_indexer *indexer = new file_indexer(dir);
        indexer->moveToThread(workerThread);

        connect(workerThread, &QThread::started,
                indexer, &file_indexer::startIndexing);
        connect(indexer, &file_indexer::onComplete,
                this, &MainWindow::onIndexComplete);
        connect(indexer, &file_indexer::onFileIndexed,
                this, &MainWindow::receiveIndexedFile);

        connect(workerThread, &QThread::finished,
                indexer, &QObject::deleteLater);
        connect(indexer, &file_indexer::onError,
                this, &MainWindow::receiveError);

        workerThread->start();
    }
}

void MainWindow::stopIndexing() {
    interruptWorkers();

    ui->buttonIndex->setText("Start indexing");
    ui->buttonIndex->repaint();
    connect(ui->buttonIndex, &QPushButton::clicked,
               this, &MainWindow::startIndexing);
    disconnect(ui->buttonIndex, &QPushButton::clicked,
            this, &MainWindow::stopIndexing);

    updateStatusBar();
    ui->progressBar->setValue(0);

    ui->buttonSearch->setDisabled(false);
    ui->buttonSearch->repaint();
}

void MainWindow::onIndexComplete(QString const &dir) {
    if (_dirs[dir] != nullptr) {
        _dirs[dir]->setTextColor(QColor(0, 50, 0));
        _unindexed_amount -= _unindexed_dirs[dir];
    }
    _unindexed_dirs.remove(dir);
    if (_unindexed_dirs.empty()) {
        ui->buttonIndex->setDisabled(true);
        stopIndexing();
    }
}

void MainWindow::startSearching() {
    ui->buttonIndex->setDisabled(true);
    ui->buttonIndex->repaint();

    ui->buttonSearch->setText("Abort searching");
    ui->buttonSearch->repaint();

    disconnect(ui->buttonSearch, &QPushButton::clicked,
               this, &MainWindow::startSearching);
    connect(ui->buttonSearch, &QPushButton::clicked,
            this, &MainWindow::stopSearching);

    QString substring = ui->lineEdit->text();
    ui->plainTextEdit_Error->appendPlainText(substring);

    QThread *workerThread = requestNewThread();
    string_finder *finder = new string_finder(_file_indexes, substring);
    finder->moveToThread(workerThread);

    connect(workerThread, &QThread::started,
            finder, &string_finder::startScanning);
    connect(finder, &string_finder::onInstanceLocated,
            this, &MainWindow::receiveInstance);
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
    interruptWorkers();

    ui->buttonSearch->setText("Start searching");
    ui->buttonSearch->repaint();
    connect(ui->buttonSearch, &QPushButton::clicked,
            this, &MainWindow::startSearching);
    disconnect(ui->buttonSearch, &QPushButton::clicked,
               this, &MainWindow::stopSearching);

    updateStatusBar();
    resetButtonIndex();
}

void MainWindow::onSearchComplete() {
    stopSearching();
}

void MainWindow::receiveIndexedFile(const QString &file_name, const file_index &index) {
    ui->progressBar->setValue(ui->progressBar->value() + 1);
    _file_indexes.insert(file_name, index);
}

void MainWindow::receiveInstance(const QString &file_name, const QVector<QPair<quint64, QString>> &positions) {
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, file_name);
    item->setText(1, QString::number(positions.size()));

    for (auto const &pos : positions) {
        QTreeWidgetItem *child = new QTreeWidgetItem(item);
        child->setText(0, pos.second);
        child->setText(1, QString::number(pos.first));
    }
}

void MainWindow::receiveError(QString const &error) {
    ui->plainTextEdit_Error->appendHtml(error);
}

MainWindow::~MainWindow() {
    interruptWorkers();
}
