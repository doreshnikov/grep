#ifndef FILE_INDEX_H
#define FILE_INDEX_H

#include <QObject>

class file_index {

public:

    file_index();
    ~file_index();

    file_index(file_index const &) = default;
    file_index &operator=(file_index const &) = default;

    quint64 get_id() const;

    static quint64 ID;

private:

    quint64 _id;

};

#endif // FILE_INDEX_H
