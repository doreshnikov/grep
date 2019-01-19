#ifndef STRING_FINDER_H
#define STRING_FINDER_H

#include <QObject>
#include <QHash>
#include <QVector>
#include <QString>

#include "file_index.h"

class string_finder : public QObject {
    Q_OBJECT

public:

    string_finder(QHash<QString, file_index> const &, QString const &);
    ~string_finder();

signals:

    void onInstanceLocated(QString const &, QVector<QPair<quint64, QString>> const &);

    void onComplete();
    void onError(QString const &);

public slots:

    void startScanning();

};

#endif // STRING_FINDER_H
