QT += core xml
QT -= gui

CONFIG += c++11

TARGET = dtk-settings
CONFIG += console link_pkgconfig
CONFIG -= app_bundle
PKGCONFIG += gsettings-qt

TEMPLATE = app

SOURCES += main.cpp

isEmpty(PREFIX){
    PREFIX = /usr
}
isEmpty(BIN_INSTALL_DIR) {
    BIN_INSTALL_DIR=$${PREFIX}/lib/dtk2
}
!isEmpty(DTK_STATIC_LIB){
    DEFINES += DTK_STATIC_LIB
}

target.path = $${BIN_INSTALL_DIR}

script.files += $${PWD}/../script/*.py
script.path = $${BIN_INSTALL_DIR}

INSTALLS += target script

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release/ -ldtkcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug/ -ldtkcore
else:unix: LIBS += -L$$OUT_PWD/../../src/ -ldtkcore

INCLUDEPATH += $$PWD/../../src
DEPENDPATH += $$PWD/../../src
