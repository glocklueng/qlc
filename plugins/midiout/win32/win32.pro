include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = midiout

INCLUDEPATH += ../common
INCLUDEPATH += ../../interfaces
DEPENDPATH  += ../common
CONFIG      += plugin
LIBS        += -lwinmm

FORMS += ../common/configuremididevice.ui \
         ../common/configuremidiout.ui

SOURCES += ../common/configuremididevice.cpp \
           ../common/configuremidiout.cpp \
           mididevice.cpp \
           midiout.cpp

HEADERS += ../common/configuremididevice.h \
           ../common/configuremidiout.h \
           ../common/midiprotocol.h \
           mididevice.h \
           midiout.h

HEADERS += ../../interfaces/qlcoutplugin.h

TRANSLATIONS += MIDI_Output_fi_FI.ts
TRANSLATIONS += MIDI_Output_de_DE.ts
TRANSLATIONS += MIDI_Output_es_ES.ts
TRANSLATIONS += MIDI_Output_fr_FR.ts
TRANSLATIONS += MIDI_Output_it_IT.ts

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
