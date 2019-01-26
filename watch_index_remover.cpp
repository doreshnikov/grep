#include "watch_index_remover.h"

#include <QDirIterator>
#include <QThread>

watch_index_remover::watch_index_remover(QString const &path) : _root(path) {}

watch_index_remover::~watch_index_remover() {}

void watch_index_remover::startRemoving() {
    QDirIterator it(_root.absolutePath(), QDir::Hidden | QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDirIterator::Subdirectories);
    bool finished = true;

    while (it.hasNext()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            finished = false;
            break;
        }
        QFileInfo file_info(it.next());

        if (file_info.isFile()) {
            emit onFileMet(file_info.absoluteFilePath());
        } else if (file_info.isDir()) {
            emit onDirectoryMet(file_info.absolutePath());
        }
    }

    if (finished) {
        emit onComplete(_root.absolutePath());
    }

    QThread::currentThread()->quit();
}
