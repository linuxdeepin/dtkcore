include($$PWD/lib.pri)

QT -= gui
QT += dbus

TARGET = dtkcore

DEFINES += LIBDTKCORE_LIBRARY


INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/dtkcore_global.h \

include($$PWD/base/base.pri)
include($$PWD/log/log.pri)
include($$PWD/filesystem/filesystem.pri)

include($$PWD/install.pri)

