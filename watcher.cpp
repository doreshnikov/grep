#include "watcher.h"

#include <QDirIterator>

watcher::watcher(): _file_watcher() {
    QObject::connect(&_file_watcher, &QFileSystemWatcher::fileChanged,
                     this, &watcher::onFileChanged);
    QObject::connect(&_file_watcher, &QFileSystemWatcher::directoryChanged,
                     this, &watcher::onDirectoryChanged);
}

watcher::~watcher() {}

void watcher::add_path(const QString &path) {
    _file_watcher.addPath(path);
}

void watcher::remove_path(const QString &path) {
    _file_watcher.removePath(path);
}
