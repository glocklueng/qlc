include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = bus_test

QT      += testlib xml
CONFIG  -= app_bundle

DEPENDPATH   += ../../src
INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += bus_test.cpp
HEADERS += bus_test.h
