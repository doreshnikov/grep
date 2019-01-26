#include "file_indexer.h"

#include <QSet>
#include <QThread>
#include <QDirIterator>
#include <QTextStream>

file_indexer::file_indexer(QString const &dir) : _root(dir) {}

file_indexer::~file_indexer() {}

void file_indexer::startIndexing() {
    if (QFileInfo(_root.absolutePath()).isFile()) {
        file_index index(_root.absolutePath());
        index_file(index);
        emit onFileIndexed(index);

        emit onComplete("");
    } else {
        QDirIterator it(_root.absolutePath(), QDir::Hidden | QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
        bool finished = true;

        while (it.hasNext()) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                finished = false;
                break;
            }
            QFileInfo file_info(it.next());

            if (file_info.isFile()) {
                file_index index(file_info.absoluteFilePath());
                index_file(index);
                emit onFileIndexed(index);
            }
        }

        if (finished) {
            emit onComplete(_root.absolutePath());
        }
    }

    QThread::currentThread()->quit();
}

void file_indexer::index_file(file_index &index) {
    QFile file(index.get_file_path());

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        char buffer[BUFFER_SIZE];
        std::size_t start = 0, end = 0;

        while ((end = start + file.read(buffer + start, BUFFER_SIZE - start)) != start) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                index.clear();
                return;
            }
            if (start == 0) {
                start = 2;
            }
            for (std::size_t i = 0; i < end - 2; i++) {
                if (buffer[i] == '\0') {
                    index.clear();
                    return;
                }

                index.insert(file_index::get_trigram(buffer[i], buffer[i + 1], buffer[i + 2]));
            }

            buffer[0] = buffer[end - 2];
            buffer[1] = buffer[end - 1];

            if (index.size() > BINARY_INDEX_SIZE) {
                index.clear();
                break;
            }
        }
    } else {
        emit onError(QString("can't open file %1").arg(index.get_file_path()));
    }
}
