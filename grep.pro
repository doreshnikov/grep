#-------------------------------------------------
#
# Project created by QtCreator 2018-12-22T02:15:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fdupes
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++17

SOURCES += main.cpp\
        mainwindow.cpp \
    file_counter.cpp \
    file_indexer.cpp \
    file_index.cpp \
    string_finder.cpp \
    watcher.cpp

HEADERS  += mainwindow.h \
    file_counter.h \
    file_indexer.h \
    file_index.h \
    string_finder.h \
    watcher.h

FORMS    += mainwindow.ui
