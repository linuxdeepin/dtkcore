QT += testlib
QT -= gui

TARGET = tst_ddesktopentrytest
TEMPLATE = app
CONFIG += c++11
CONFIG -= app_bundle

!isEmpty(DTK_STATIC_LIB){
    DEFINES += DTK_STATIC_LIB
}

load(dtk_testcase)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release/ -ldtkcore -lgtest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug/ -ldtkcore -lgtest
else:unix: LIBS += -L$$OUT_PWD/../../src/ -ldtkcore -lgtest

INCLUDEPATH += $$PWD/../../src
DEPENDPATH += $$PWD/../../src
unix:QMAKE_RPATHDIR += $$OUT_PWD/../../src

QMAKE_LFLAGS += -Wl,--export-dynamic

SOURCES += \
    $$PWD/../../src/ddesktopentry.cpp \
    ut_ddesktopentrytest.cpp
