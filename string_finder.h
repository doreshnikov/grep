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

    void onInstancesFound(QString const &, QVector<QString> const &);

    void onComplete();
    void onError(QString const &);

public slots:

    void startScanning();

private:

    void scan_file(QString const &);

    QHash<QString, file_index> const &_indexes;
    QString _substring;

};

#endif // STRING_FINDER_H
