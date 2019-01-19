#ifndef FILE_INDEXER_H
#define FILE_INDEXER_H

#include "file_index.h"

#include <QObject>
#include <QDir>

class file_indexer : public QObject {
    Q_OBJECT

public:

    file_indexer(QString const &);

signals:

    void onFileIndexed(QString const &, file_index);
    void onComplete(QString const &);

    void onError(QString);

public slots:

    void startIndexing();

private:

    QDir _root;

};

#endif // FILE_INDEXER_H
