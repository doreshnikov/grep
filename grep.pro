#-------------------------------------------------
#
# Project created by QtCreator 2018-12-22T02:15:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = grep
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++17

SOURCES += main.cpp\
        mainwindow.cpp \
    file_counter.cpp \
    file_indexer.cpp \
    file_index.cpp \
    string_finder.cpp \
    watcher.cpp \
    watch_index_remover.cpp

HEADERS  += mainwindow.h \
    file_counter.h \
    file_indexer.h \
    file_index.h \
    string_finder.h \
    watcher.h \
    watch_index_remover.h

FORMS    += mainwindow.ui
