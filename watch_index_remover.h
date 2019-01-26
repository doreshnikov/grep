#ifndef WATCH_REMOVER_H
#define WATCH_REMOVER_H

#include <QObject>
#include <QString>
#include <QDir>

class watch_index_remover : public QObject {
    Q_OBJECT

public:

    watch_index_remover(QString const &);
    ~watch_index_remover();

signals:

    void onFileMet(QString const &);
    void onDirectoryMet(QString const &);
    void onComplete(QString const &);

    void onError(QString const &);

public slots:

    void startRemoving();

private:

    QDir _root;

};

#endif // WATCH_REMOVER_H
