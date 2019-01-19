#include "file_index.h"

quint64 file_index::ID = 0;

Q_DECLARE_METATYPE(file_index)

file_index::file_index() : _id(file_index::ID++) {}

file_index::~file_index() {}

quint64 file_index::get_id() const {
    return _id;
}
