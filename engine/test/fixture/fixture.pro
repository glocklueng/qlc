include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = fixture_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += fixture_test.cpp
HEADERS += fixture_test.h