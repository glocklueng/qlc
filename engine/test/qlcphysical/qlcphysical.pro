include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcphysical_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlcphysical_test.cpp
HEADERS += qlcphysical_test.h