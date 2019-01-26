#ifndef FILE_INDEX_H
#define FILE_INDEX_H

#include <QObject>
#include <QString>
#include <QVector>

#include <unordered_set>

class file_index {

public:

    file_index();
    file_index(QString const &);
    ~file_index();

    QString const &get_file_path() const;

    void insert(quint32);
    bool contains(quint32) const;
    std::size_t size() const;
    bool empty() const;
    void clear();

    static quint32 get_trigram(unsigned char, unsigned char, unsigned char);

private:

    QString _file_path;
    std::unordered_set<quint32> _trigrams;

};

#endif // FILE_INDEX_H
