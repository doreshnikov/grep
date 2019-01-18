#ifndef TESTS_H
#define TESTS_H

#include <initializer_list>
#include <string>
#include <memory>

#include <QObject>
#include <QString>
#include <QVector>
#include <QSet>
#include <QThread>

#include "../duplicates_scanner.h"

class test : public QObject {
    Q_OBJECT

public:

    static qint64 now();

    test(QString const &);
    virtual ~test();

    virtual void generate() = 0;
    virtual void clean();

    QString get_name() const;
    QString get_dir() const;

protected:

    QString _name;
    QDir _dir;

};

class basic_test : public test {

public:

    basic_test(QString const &, std::initializer_list<QString> const &);
    ~basic_test();

    void generate() override;

private:

    QVector<QString> _files_data;

};

class script_test : public test {

public:

    script_test(QString const &, std::string const &);
    ~script_test();

    void generate() override;
    void clean() override;

private:

    std::string _script_path;
    qint64 _time;

};

class subdirectory_test : public test {

public:

    subdirectory_test(QString const &, std::size_t, std::initializer_list<QString> const &);
    ~subdirectory_test();

    void generate() override;

private:

    QVector<QString> _files_data;
    std::size_t _depth;

};

class duplicates_scanner_tester : public QObject {
    Q_OBJECT

public:

    duplicates_scanner_tester();
    ~duplicates_scanner_tester();

    void add_test(test *, std::initializer_list<int> const &, std::initializer_list<QString> const &);
    void run_all();

signals:

    void onComplete();

public slots:

    void receiveDuplicatesBucket(QVector<QString> const &);
    void receiveError(QString const &);

private:

    struct full_test {
        std::shared_ptr<test> _t;
        QVector<int> _bucket_sizes;
        QSet<QString> _errors;

        full_test();
        full_test(test *, QVector<int> const &, QSet<QString> const &);
        ~full_test();

        full_test(full_test const &other);
    };

    duplicates_scanner *_scanner;
    QThread *_workerThread;

    QVector<full_test> _tests;
    int _test_id;
    QVector<int> _bucket_sizes;
    QSet<QString> _errors;

    std::size_t _success;

};

#endif // TESTS_H
