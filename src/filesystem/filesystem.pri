include($$PWD/private/private.pri)

HEADERS += \
    $$PWD/dbasefilewatcher.h \
    $$PWD/dfilesystemwatcher.h \
    $$PWD/dfilewatcher.h \
    $$PWD/dfilewatchermanager.h \
    $$PWD/dpathbuf.h

SOURCES += \
    $$PWD/dbasefilewatcher.cpp \
    $$PWD/dfilewatcher.cpp \
    $$PWD/dfilewatchermanager.cpp


linux {
    CONFIG += link_pkgconfig
    PKGCONFIG += gsettings-qt

    SOURCES += \
        $$PWD/dfilesystemwatcher_linux.cpp
} else:win* {
    SOURCES += \
        $$PWD/dfilesystemwatcher_win.cpp
} else:mac* {
    SOURCES += \
        $$PWD/dfilesystemwatcher_win.cpp
}

includes.files += $$PWD/*.h
includes.files += $$PWD/*.cpp
includes.files += \
    $$PWD/DFileWatcher \
    $$PWD/DBaseFileWatcher \
    $$PWD/DFileSystemWatcher \
    $$PWD/DFileWatcherManager \
    $$PWD/DPathBuf
