#ifndef WATCHER_H
#define WATCHER_H

#include <QObject>
#include <QString>
#include <QFileSystemWatcher>

class watcher : public QObject {
    Q_OBJECT

public:

    watcher();
    ~watcher();

    void add_path(QString const &);
    void remove_path(QString const &);

signals:

    void onFileChanged(QString const &);
    void onDirectoryChanged(QString const &);

private:

    QFileSystemWatcher _file_watcher;

};

#endif // WATCHER_H
