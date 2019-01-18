#ifndef FILE_COUNTER_H
#define FILE_COUNTER_H

#include <QObject>
#include <QString>
#include <QDir>

class file_counter : public QObject {
    Q_OBJECT

public:

    file_counter(QString const &);
    ~file_counter();

signals:

    void onComplete(int, qint64);

public slots:

    void startCounting();

private:

    QDir _root;

};

#endif // FILE_COUNTER_H
