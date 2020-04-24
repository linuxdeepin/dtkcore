QT -= gui
TEMPLATE = app
CONFIG += qt

HEADERS += ../../src/dsysinfo.h \
    ../../src/ddesktopentry.h

SOURCES += \
    main.cpp \
    ../../src/dsysinfo.cpp \
    ../../src/ddesktopentry.cpp

INCLUDEPATH += ../../src
DESTDIR = $$_PRO_FILE_PWD_/../../bin

DTK_MODULE_NAME=dtkcore
include(../../dtk_build_config.prf)
target.path = $$TOOL_INSTALL_DIR

INSTALLS += target
