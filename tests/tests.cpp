#include <iostream>
#include <algorithm>
#include <chrono>

#include <QCoreApplication>
#include <QtDebug>

#include "tests.h"

namespace {

QString
    DIR = "tmp",

    empty = "",
    a1 = "a",
    b1 = "b",

    a256 = QString("a").repeated(256),
    b256 = QString("b").repeated(256),

    a257 = a256 + "a",
    a256b = a256 + "b",
    a256c = a256 + "c",

    rnd1 = "abacabadabacaba",
    rnd2 = "adaafalxknlnlxnn.kn.snldnflsnlknsldnfk;f;aj;fj;aj;wj;aj;wf\0\1afwfa",
    rnd3 = QString("abacabadabacaba").repeated(100),
    rnd4 = QString("abacabadabacaba").repeated(50) + "xyzcv";

}

qint64 test::now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

test::test(QString const &name) : _name(name), _dir(DIR) {
    _dir.mkdir(_name);
    _dir = QDir(_dir.filePath(_name));
}

test::~test() {}

void test::clean() {
    _dir.removeRecursively();
    QDir(_dir.filePath("..")).rmdir(_name);
}

QString test::get_name() const {
    return _name;
}

QString test::get_dir() const {
    return _dir.dirName();
}

basic_test::basic_test(QString const &name, std::initializer_list<QString> const &list) : test(name), _files_data(list) {}

basic_test::~basic_test() {}

void basic_test::generate() {
    qDebug().noquote() << QString("[Generating  %1 ...]").arg(_name);

    std::size_t id = 0;
    for (auto const &file_data : _files_data) {
        QFile file(_dir.filePath(QString::number(id++)));
        if (file.open(QFile::ReadWrite)) {
            file.write(file_data.toUtf8());
        }
    }
}

script_test::script_test(QString const &name, std::string const &script_path) : test(name), _script_path(script_path) {}

script_test::~script_test() {}

void script_test::generate() {
    qDebug().noquote() << QString("[Running script %1 ...]").arg(_name);
    _dir = QDir(QDir(QCoreApplication::applicationDirPath()).filePath(QString("%1test").arg(QString::fromStdString(_script_path))));
    system((QDir(QCoreApplication::applicationDirPath()).filePath(_script_path.data()).toStdString() + " >/dev/null 2>/dev/null").data());
    _time = test::now();
}

void script_test::clean() {
    qDebug().noquote() << QString("    - time: %1ms").arg(test::now() - _time);
    _dir.removeRecursively();
    QDir(_dir.filePath("..")).rmdir(_dir.dirName());
}

subdirectory_test::subdirectory_test(const QString &name, std::size_t depth, const std::initializer_list<QString> &list) : test(name), _depth(depth), _files_data(list) {}

subdirectory_test::~subdirectory_test() {}

void subdirectory_test::generate() {
    qDebug().noquote() << QString("[Generating subdirectories %1 ...]").arg(_name);

    QDir root = _dir;
    std::size_t id = 0;
    for (auto const &file_data : _files_data) {
        for (std::size_t d = 0; d <= _depth; d++) {
            QFile file(_dir.filePath(QString::number(id++)));
            if (file.open(QFile::ReadWrite)) {
                file.write(file_data.toUtf8());
            }
            _dir.mkdir(QString::number(id));
            _dir = QDir(_dir.filePath(QString::number(id++)));
        }
        _dir = root;
    }
}

duplicates_scanner_tester::duplicates_scanner_tester() : _scanner(nullptr), _workerThread(nullptr), _test_id(0),  _bucket_sizes(), _errors(), _success(0) {}

duplicates_scanner_tester::~duplicates_scanner_tester() {
    _workerThread->requestInterruption();
    _workerThread->quit();
    delete _workerThread;
    delete _scanner;
}

duplicates_scanner_tester::full_test::full_test() : _t(nullptr), _bucket_sizes(), _errors() {}

duplicates_scanner_tester::full_test::full_test(test *t, QVector<int> const &bucket_sizes, QSet<QString> const &errors) : _t(t), _bucket_sizes(bucket_sizes), _errors(errors) {}

duplicates_scanner_tester::full_test::~full_test() {}

duplicates_scanner_tester::full_test::full_test(const full_test &other) : _t(other._t), _bucket_sizes(other._bucket_sizes), _errors(other._errors) {}

void duplicates_scanner_tester::add_test(test *t, std::initializer_list<int> const &expect, std::initializer_list<QString> const &error) {
    QVector<int> bucket_sizes;
    for (int size : expect) {
        bucket_sizes.append(size);
    }
    QSet<QString> errors;
    for (auto const &e : error) {
        errors.insert(e);
    }
    _tests.append(full_test(t, bucket_sizes, errors));
}

void duplicates_scanner_tester::run_all() {
    for (_test_id = 0; _test_id < _tests.size(); _test_id++) {
        std::shared_ptr<test> t = _tests[_test_id]._t;

        _scanner = new duplicates_scanner(QDir(DIR).filePath(t->get_dir()));
        _workerThread = new QThread();
        _scanner->moveToThread(_workerThread);
        connect(_scanner, &duplicates_scanner::onDuplicatesBucketFound,
                this, &duplicates_scanner_tester::receiveDuplicatesBucket);
        connect(_scanner, &duplicates_scanner::onError,
                this, &duplicates_scanner_tester::receiveError);
        connect(_scanner, &duplicates_scanner::onComplete,
                _workerThread, &QThread::quit);
        _workerThread->start();

        t->generate();
        _scanner->startScanning();
        _workerThread->wait();

        QDebug debug = qDebug().noquote();
        std::sort(_bucket_sizes.begin(), _bucket_sizes.end());
        if ((_bucket_sizes == _tests[_test_id]._bucket_sizes) && (_errors == _tests[_test_id]._errors)) {
            debug << "[+] Test passed";
            _success++;
        } else {
            debug << "[-] Test failed, got:";
            debug << "\n    - bs:";
            for (int amount :_bucket_sizes) {
                debug << QString::number(amount);
            }
            debug << QString("\n    - er: %1").arg(_errors.size());
        }

        t->clean();
        _bucket_sizes.clear();
        _errors.clear();
        delete _scanner;
        delete _workerThread;
    }

    qDebug().noquote() << QString("=").repeated(15) << QString("\nPassed: %1/%2").arg(QString::number(_success)).arg(QString::number(_tests.size()));
    emit onComplete();
}

void duplicates_scanner_tester::receiveDuplicatesBucket(const QVector<QString> &bucket) {
    _bucket_sizes.append(bucket.size());
}

void duplicates_scanner_tester::receiveError(const QString &error) {
    _errors.insert(error);
}

int main(int argc, char *argv[]) {

    QCoreApplication a(argc, argv);
    duplicates_scanner_tester tester;
    QDir::current().mkdir(DIR);

    tester.add_test(
        new basic_test("No files", {}),
        {},
        {}
    );
    tester.add_test(
        new basic_test("Empty x2", {empty, empty}),
        {2},
        {}
    );
    tester.add_test(
        new basic_test("Empty x2, a1 x3, b1 x4", {empty, empty, a1, a1, a1, b1, b1, b1, b1, rnd1, rnd1, rnd2, rnd3, rnd4}),
        {2, 2, 3, 4},
        {}
    );
    tester.add_test(
        new basic_test("256+ symbols", {a256, b256, b256, b256, a256b, a256c, a257, a256c, a257}),
        {2, 2, 3},
        {}
    );
    tester.add_test(
        new basic_test("Random trash", {rnd1, rnd2, rnd3, rnd4, rnd1, rnd2, rnd4, rnd2, rnd4, rnd2}),
        {2, 3, 4},
        {}
    );

    tester.add_test(
        new subdirectory_test("One file", 9, {rnd4}),
        {10},
        {}
    );
    tester.add_test(
        new subdirectory_test("4 files, deep", 99, {empty, rnd2, rnd2, rnd4}),
        {100, 100, 200},
        {}
    );

    tester.add_test(
        new script_test("300 diff files, 2Kb", "300diff"),
        {},
        {}
    );

    QObject::connect(&tester, &duplicates_scanner_tester::onComplete,
                     &a, &QCoreApplication::quit);
    tester.run_all();
    QDir::current().rmdir(DIR);
    return a.exec();

}
