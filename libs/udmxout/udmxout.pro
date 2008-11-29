include(../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin warn_on release

INCLUDEPATH	+= . ../../libs/
unix:LIBS	+= -lusb

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# Forms
FORMS += configureudmxout.ui

# Headers
HEADERS += configureudmxout.h \
	   udmxout.h

win32:HEADERS += libusb_dyn.h

# Sources
SOURCES += configureudmxout.cpp \
	   udmxout.cpp

win32:SOURCES += libusb_dyn.c

