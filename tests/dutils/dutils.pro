QT += testlib dbus
QT -= gui

TEMPLATE = app
CONFIG += c++17

!isEmpty(DTK_STATIC_LIB){
    DEFINES += DTK_STATIC_LIB
}

include(../dtk_testcase.prf)

SOURCES += \
    main.cpp \
    dutiltester.cpp \
    singletontester.cpp

HEADERS += \
    dutiltester.h \
    singletontester.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release/ -ldtkcore -lgtest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug/ -ldtkcore -lgtest
else:unix: LIBS += -L$$OUT_PWD/../../src/ -ldtkcore -lgtest

INCLUDEPATH += $$PWD/../../src
DEPENDPATH += $$PWD/../../src
QMAKE_RPATHDIR += $$PWD/../../src

RESOURCES += \
    data.qrc
