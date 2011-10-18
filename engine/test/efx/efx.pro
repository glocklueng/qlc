include(../../../coverage.pri)
TEMPLATE = app
LANGUAGE = C++
TARGET   = efx_test

QT      += testlib xml
CONFIG  -= app_bundle

INCLUDEPATH  += ../../../plugins/interfaces
INCLUDEPATH  += ../mastertimer
INCLUDEPATH  += ../../src
QMAKE_LIBDIR += ../../src
LIBS         += -lqlcengine

SOURCES += efx_test.cpp ../mastertimer/mastertimer_stub.cpp
HEADERS += efx_test.h ../mastertimer/mastertimer_stub.h
