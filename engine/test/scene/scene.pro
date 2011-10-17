include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = scene_test

QT      += testlib xml

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += scene_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += scene_test.h ../mastertimer/mastertimer_stub.h