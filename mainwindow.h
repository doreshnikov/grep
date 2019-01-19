#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "file_index.h"

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
    void stopIndexing();
    void startSearching();
    void stopSearching();

    void showAboutDialog();

    void onCountComplete(QString const &, int, qint64);
    void onIndexComplete(QString const &);

    void receiveIndexedFile(QString const &, file_index const &);
    void receiveError(QString const &);

private:

    QThread *requestNewThread();
    void interruptWorkers();

    void updateStatusBar();
    void resetButtonIndex();

    std::unique_ptr<Ui::MainWindow> ui;
    QHash<QString, QListWidgetItem *> _dirs;

    QHash<QString, file_index> _file_indexes;

    QHash<QString, int> _unindexed_dirs;
    int _unindexed_amount;

    QVector<QThread *> _workerThreads;

};

#endif // MAINWINDOW_H
