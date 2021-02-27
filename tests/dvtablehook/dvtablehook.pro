QT += testlib
QT -= gui

TEMPLATE = app
CONFIG += c++11

# TODO: vtabhook release test failed
QMAKE_CXXFLAGS_RELEASE -= -O2

!isEmpty(DTK_STATIC_LIB){
    DEFINES += DTK_STATIC_LIB
}

load(dtk_testcase)

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release/ -ldtkcore -lgtest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug/ -ldtkcore -lgtest
else:unix: LIBS += -L$$OUT_PWD/../../src/ -ldtkcore -lgtest -ldl

INCLUDEPATH += $$PWD/../../src
DEPENDPATH += $$PWD/../../src
QMAKE_RPATHDIR += $$OUT_PWD/../../src

QMAKE_LFLAGS += -Wl,--export-dynamic

HEADERS += \
    $$PWD/../../src/util/dvtablehook.h

SOURCES += \
    $$PWD/../../src/util/dvtablehook.cpp \
    ut_dvtablehook.cpp
