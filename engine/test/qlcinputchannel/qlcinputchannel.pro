include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = qlcinputchannel_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += qlcinputchannel_test.cpp
HEADERS += qlcinputchannel_test.h
