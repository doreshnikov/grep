#include "string_finder.h"

#include <QThread>
#include <QFile>
#include <QTextStream>

string_finder::string_finder(const QHash<QString, file_index> &indexes, QString const &substring) : _indexes(indexes), _substring(substring) {}

string_finder::~string_finder() {}

void string_finder::startScanning() {
    if (_substring.size() < 3) {
        emit onComplete();
        QThread::currentThread()->quit();
    }

    QVector<quint32> trigrams;
    QByteArray utf8bytes = _substring.toUtf8();
    for (int i = 0; i < utf8bytes.size() - 3; i++) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }
        trigrams.append(file_index::get_trigram(utf8bytes[i], utf8bytes[i + 1], utf8bytes[i + 2]));
    }

    for (file_index const &index : _indexes.values()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        bool trigram_check = true;
        for (auto trigram : trigrams) {
            if (!index.contains(trigram)) {
                trigram_check = false;
                break;
            }
        }

        if (trigram_check) {
            scan_file(index.get_file_path());
        }
    }

    emit onComplete();
    QThread::currentThread()->quit();
}

void string_finder::scan_file(const QString &file_name) {
    QFile file(file_name);

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");

        quint32 max_size = std::max(_substring.size() - 1, 128);
        QString buffer1 = "", buffer2 = "";
        QVector<QString> result;

        bool last = true;
        while (!(buffer2 = stream.read(max_size)).isEmpty() || last) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }
            if (buffer2.isEmpty()) {
                last = false;
            }

            int index = 0;
            std::size_t global_index = 0;
            if ((index = (buffer1 + buffer2).indexOf(_substring)) != -1) {
                if (index < buffer1.size()) {
                    QString instance(buffer1 + buffer2);
                    instance = instance.right(instance.size() - index);
                    if (index != 0 || global_index != 0) {
                        instance = QString("...") + instance;
                    }
                    int next_line = instance.indexOf('\n');
                    if (next_line != -1) {
                        instance.truncate(instance.indexOf('\n'));
                    }
                    result.push_back(instance);
                }
            }

            global_index += buffer1.size();
            buffer1 = std::move(buffer2);
        }

        if (result.size() != 0) {
            emit onInstancesFound(file_name, result);
        }
    } else {
        emit onError(QString("can't open file %1").arg(file_name));
    }
}
