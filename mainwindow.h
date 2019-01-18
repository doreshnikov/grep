#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

    bool deleteOneSelected(QString const &, QString const &, QMessageBox::StandardButton &);

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void selectDirectory();
    void deleteSelected();
    void startScanning();
    void stopScanning();

    void showAboutDialog();

    void onCounted(int, qint64);

    void receiveProgress(QString const &);
    void receiveDuplicatesBucket(QVector<QString> const &);
    void receiveError(QString const &);

    void interruptWorker();

private:

    std::unique_ptr<Ui::MainWindow> ui;
    QString _dir;
    QMap<QString, QTreeWidgetItem *> _duplicates;
    QMap<QString, qint64> _duplicates_count;

    QThread *_workerThread;

};

#endif // MAINWINDOW_H
