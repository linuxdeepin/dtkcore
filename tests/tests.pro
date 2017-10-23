TEMPLATE = app
QT += testlib
QT -= gui
CONFIG += testcase c++11 link_pkgconfig
TARGET = tests
PKGCONFIG += gsettings-qt

SOURCES += \
    main.cpp \
    dutiltester.cpp \
    singletontester.cpp

HEADERS += \
    dutiltester.h \
    singletontester.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../src/release/ -ldtkcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../src/debug/ -ldtkcore
else:unix: LIBS += -L$$OUT_PWD/../src/ -ldtkcore

INCLUDEPATH += $$PWD/../src
DEPENDPATH += $$PWD/../src
QMAKE_RPATHDIR += $$PWD/../src
