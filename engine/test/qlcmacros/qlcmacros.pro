include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcmacros_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlcmacros_test.cpp
HEADERS += qlcmacros_test.h
