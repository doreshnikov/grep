#include "file_indexer.h"

#include <QThread>
#include <QDirIterator>

file_indexer::file_indexer(QString const &dir) : _root(dir) {}

void file_indexer::startIndexing() {
    QDirIterator it(_root.absolutePath(), QDir::Hidden | QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
    bool finished = true;

    while (it.hasNext()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            finished = false;
            break;
        }
        QFileInfo file_info(it.next());

        if (file_info.isFile()) {
            emit onFileIndexed(file_info.absoluteFilePath(), file_index());
            QThread::currentThread()->sleep(1);
        }
    }

    if (finished) {
        emit onComplete(_root.absolutePath());
    }

    QThread::currentThread()->quit();
}
