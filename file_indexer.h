#ifndef FILE_INDEXER_H
#define FILE_INDEXER_H

#include "file_index.h"

#include <QObject>
#include <QDir>

class file_indexer : public QObject {
    Q_OBJECT

public:

    static size_t const BUFFER_SIZE = 1024 * 1024;
    static size_t const BINARY_INDEX_SIZE = 20000;

    file_indexer(QString const &);
    ~file_indexer();

signals:

    void onFileIndexed(file_index const &);
    void onComplete(QString const &);

    void onError(QString);

public slots:

    void startIndexing();

private:

    void index_file(file_index &);

    QDir _root;

};

#endif // FILE_INDEXER_H
