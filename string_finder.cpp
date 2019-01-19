#include "string_finder.h"

#include <QThread>

string_finder::string_finder(const QHash<QString, file_index> &indexes, const QString &substring) {}

string_finder::~string_finder() {}

void string_finder::startScanning() {
    emit onInstanceLocated(QString("located"), QVector<QPair<quint64, QString>>{qMakePair(1, QString("pos"))});
    emit onComplete();

    QThread::currentThread()->quit();
}
