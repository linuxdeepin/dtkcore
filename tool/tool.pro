QT += core
QT -= gui

CONFIG += c++11

TARGET = dtk-settings-tool
CONFIG += console link_pkgconfig
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

isEmpty(PREFIX){
    PREFIX = /usr
}

binary.files += $${OUT_PWD}/dtk-settings-tool
binary.path = $${PREFIX}/bin

INSTALLS += binary

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../src/release/ -ldtkcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../src/debug/ -ldtkcore
else:unix: LIBS += -L$$OUT_PWD/../src/ -ldtkcore

INCLUDEPATH += $$PWD/../src
DEPENDPATH += $$PWD/../src
