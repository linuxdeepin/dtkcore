include($$PWD/lib.pri)
include($$PWD/install.pri)

QT -= gui

TARGET = dtkcore

DEFINES += LIBDTKCORE_LIBRARY


INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/dtkcore_global.h \

include($$PWD/base/dbase.pri)
