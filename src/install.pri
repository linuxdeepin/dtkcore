includes.path = $${DTK_INCLUDEPATH}/DCore
includes.files += $$PWD/base/*.h
includes.files += $$PWD/base/*.cpp
includes.files += \
    $$PWD/base/DObject \
    $$PWD/base/DSingleton

includes.files += $$PWD/log/*.h
includes.files += $$PWD/log/*.cpp
includes.files += \
    $$PWD/log/DLog

INSTALLS += includes target


QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

QMAKE_PKGCONFIG_NAME = DTK_CORE
QMAKE_PKGCONFIG_DESCRIPTION = Deepin Tool Kit Core Header Files
QMAKE_PKGCONFIG_INCDIR = $$includes.path
