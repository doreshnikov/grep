#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "file_index.h"
#include "watcher.h"

#include <mutex>
#include <memory>
#include <QMainWindow>
#include <QMap>
#include <QThread>
#include <QFileInfo>
#include <QFileInfoList>

#include <QtWidgets>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:

    void selectDirectory();
    void startIndexing();
    void startSearching();

    void stopIndexing();
    void stopSearching();

    void onCountComplete(QString const &, int, qint64);
    void onIndexComplete(QString const &);
    void onSearchComplete();

    void onFileChanged(QString const &);
    void onDirectoryChanged(QString const &);

    void receiveIndexedFile(file_index const &);
    void receiveReindexedFile(file_index const &);
    void receiveInstances(QString const &, QVector<QString> const &);
    void receiveError(QString const &);

    void removeDirectory(QListWidgetItem *);
    void onRemovingComplete(QString const &);
    void onIndexedFileRemoved(QString const &);
    void onIndexedDirectoryRemoved(QString const &);
    void stopRemoving();

    void showAboutDialog();

private:

    QThread *request_new_thread();
    void interrupt_workers();

    void update_status_bar();
    void reset_index_button();
    void reindex(QString const &);

    std::unique_ptr<Ui::MainWindow> ui;
    QHash<QString, QListWidgetItem *> _dirs;

    QHash<QString, file_index> _file_indexes;
    std::mutex _file_indexes_mutex;
    QHash<QString, int> _unindexed_dirs;
    int _unindexed_amount;

    QVector<QThread *> _worker_threads;
    QThread *_watcher_thread;
    std::unique_ptr<watcher> _watcher;

};

#endif // MAINWINDOW_H
