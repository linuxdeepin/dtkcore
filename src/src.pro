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
include($$PWD/settings/settings.pri)

# ----------------------------------------------
# install config

includes.files += $$PWD/*.h
includes.path = $${DTK_INCLUDEPATH}/DCore

INSTALLS += includes target

QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

QMAKE_PKGCONFIG_NAME = DTK_CORE
QMAKE_PKGCONFIG_DESCRIPTION = Deepin Tool Kit Core Header Files
QMAKE_PKGCONFIG_INCDIR = $$includes.path


