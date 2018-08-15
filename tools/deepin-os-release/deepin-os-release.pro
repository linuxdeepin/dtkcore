QT -= gui
TEMPLATE = app
CONFIG += qt

HEADERS += ../../src/dsysinfo.h

SOURCES += \
    main.cpp \
    ../../src/dsysinfo.cpp

INCLUDEPATH += ../../src
DESTDIR = $$_PRO_FILE_PWD_/../../bin
