#-----------------------------------------
#
# Project created not by QtCreator fuck it
#
#-----------------------------------------

QT += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testing
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++17

SOURCES += ../duplicates_scanner.cpp \
           tests.cpp

HEADERS += ../duplicates_scanner.h \
           tests.h
