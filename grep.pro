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
    duplicates_scanner.cpp \
    file_counter.cpp

HEADERS  += mainwindow.h \
    duplicates_scanner.h \
    file_counter.h

FORMS    += mainwindow.ui
