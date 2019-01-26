#include "file_index.h"

#include <QFile>

Q_DECLARE_METATYPE(file_index)

inline quint32 num(unsigned char c) {
    return static_cast<quint32>(static_cast<int>(c));
}

quint32 file_index::get_trigram(unsigned char c1, unsigned char c2, unsigned char c3) {
    quint32 n1 = num(c1), n2 = num(c2), n3 = num(c3);
    return (n1 << (16 * sizeof(char))) | (n2 << (8 * sizeof(char))) | (n3);
}

file_index::file_index() : _file_path(""), _trigrams() {}

file_index::file_index(QString const &file_path) : _file_path(file_path), _trigrams() {}

file_index::~file_index() {}

QString const &file_index::get_file_path() const {
    return _file_path;
}

void file_index::insert(quint32 trigram) {
    _trigrams.insert(trigram);
}

bool file_index::contains(quint32 trigram) const {
    return _trigrams.count(trigram) != 0;
}

std::size_t file_index::size() const {
    return _trigrams.size();
}

bool file_index::empty() const {
    return _trigrams.size() == 0;
}

void file_index::clear() {
    _trigrams.clear();
}
