QT += testlib
QT -= gui

TEMPLATE = app
CONFIG += c++11

!isEmpty(DTK_STATIC_LIB){
    DEFINES += DTK_STATIC_LIB
}

include(../dtk_testcase.prf)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release/ -ldtkcore -lgtest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug/ -ldtkcore -lgtest
else:unix: LIBS += -L$$OUT_PWD/../../src/ -ldtkcore -lgtest

INCLUDEPATH += $$PWD/../../src
DEPENDPATH += $$PWD/../../src
QMAKE_RPATHDIR += $$OUT_PWD/../../src

QMAKE_LFLAGS += -Wl,--export-dynamic

SOURCES += \
    gts_dvtablehook.cpp
