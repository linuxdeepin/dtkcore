QT += core xml
QT -= gui

CONFIG += c++11

TARGET = dtk-settings
CONFIG += console link_pkgconfig
CONFIG -= app_bundle
PKGCONFIG += gsettings-qt

TEMPLATE = app

SOURCES += main.cpp

!isEmpty(DTK_STATIC_LIB){
    DEFINES += DTK_STATIC_LIB
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release/ -ldtkcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug/ -ldtkcore
else:unix: LIBS += -L$$OUT_PWD/../../src/ -ldtkcore

INCLUDEPATH += $$PWD/../../src \
               $$PWD/../../src/settings
DEPENDPATH += $$PWD/../../src
DESTDIR = $$_PRO_FILE_PWD_/../../bin

DTK_MODULE_NAME=dtkcore
load(dtk_build_config)
target.path = $$TOOL_INSTALL_DIR

scripts.files += ../script/*.py
scripts.path = $$TOOL_INSTALL_DIR

INSTALLS += target scripts
