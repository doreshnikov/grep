#include "duplicates_scanner.h"

#include <QThread>
#include <QDirIterator>
#include <QHash>
#include <QVector>
#include <QCryptographicHash>

#include <QtDebug>

duplicates_scanner::duplicates_scanner(QString const &dir) : _root(dir) {}

duplicates_scanner::~duplicates_scanner() {}

bool duplicates_scanner::files_are_equal(const QString &origin, const QString &other) {
    QFile f1(origin), f2(other);
    if (!f1.open(QFile::ReadOnly)) {
        emit onError(QString("can't open file %1 for comparison with %2").arg(origin).arg(other));
        return false;
    }
    if (!f2.open(QFile::ReadOnly)) {
        emit onError(QString("can't open file %1 for comparison with %2").arg(other).arg(origin));
        return false;
    }

    QByteArray buffer1(256, 0), buffer2(256, 0);
    while (!f1.atEnd()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return false;
        }
        f1.read(buffer1.data(), 256);
        f2.read(buffer2.data(), 256);
        if (buffer1 != buffer2) {
            return false;
        }
    }

    f1.close();
    f2.close();
    return true;
}

void duplicates_scanner::startScanning() {
    QHash<qint64, QVector<QString>> buckets_by_size;

    {
        QDirIterator it(_root.absolutePath(), QDir::Hidden | QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QFileInfo file_info(it.next());

            if (file_info.isFile()) {
                if (!buckets_by_size.contains(file_info.size())) {
                    buckets_by_size[file_info.size()] = QVector<QString>();
                }
                buckets_by_size[file_info.size()].append(file_info.filePath());
            }
        }
    }

    for (auto &bucket : buckets_by_size) {
        if (bucket.size() < 2) {
            for (QString const &file_name : bucket) {
                emit onFileProcessed(file_name);
            }
            continue;
        }

        QHash<QByteArray, QVector<QString>> origins;
        QHash<QString, QVector<QString>> duplicates;
        for (auto const &file_name : bucket) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            QFile file(file_name);
            if (!file.open(QFile::ReadOnly)) {
                emit onError(QString("can't open file %1").arg(file_name));
                emit onFileProcessed(file_name);
                continue;
            }

            QByteArray hash(256, 0);
            file.read(hash.data(), 256);
            file.close();

            if (origins.contains(hash)) {
                bool found = false;
                for (auto const &origin : origins[hash]) {
                    if (files_are_equal(origin, file_name)) {
                        duplicates[origin].append(file_name);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    origins[hash].append(file_name);
                    duplicates.insert(file_name, QVector<QString>{file_name});
                }
            } else {
                origins.insert(hash, QVector<QString>{file_name});
                duplicates.insert(file_name, QVector<QString>{file_name});
            }

            emit onFileProcessed(file_name);
        }

        for (auto const &duplicates_bucket : duplicates) {
            if (duplicates_bucket.size() < 2) {
                continue;
            }
            emit onDuplicatesBucketFound(duplicates_bucket);
        }
    }

    emit onComplete();

    QThread::currentThread()->quit();
}
