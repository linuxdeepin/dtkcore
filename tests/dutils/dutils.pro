QT += testlib dbus
QT -= gui

TEMPLATE = app
CONFIG += c++11

!isEmpty(DTK_STATIC_LIB){
    DEFINES += DTK_STATIC_LIB
}
# 使用 tmp 目录下的 os-version
DEFINES += OS_VERSION_TEST_FILE=\\\"/tmp/etc/os-version\\\"

load(dtk_testcase)

SOURCES += \
    $$PWD/../../src/util/dtimeunitformatter.cpp \
    $$PWD/../../src/util/ddisksizeformatter.cpp \
    $$PWD/../../src/log/LogManager.cpp \
    $$PWD/../../src/filesystem/dpathbuf.cpp \
    $$PWD/../../src/util/ddbussender.cpp \
    $$PWD/../../src/settings/dsettings.cpp \
    $$PWD/../../src/settings/dsettingsgroup.cpp \
    $$PWD/../../src/settings/dsettingsoption.cpp \
    $$PWD/../../src/dsysinfo.cpp \
    main.cpp \
    ut_dutil.cpp \
    ut_singleton.cpp

HEADERS += \
    $$PWD/../../src/util/dtimeunitformatter.h \
    $$PWD/../../src/util/ddisksizeformatter.h \
    $$PWD/../../src/log/LogManager.h \
    $$PWD/../../src/filesystem/dpathbuf.h \
    $$PWD/../../src/util/ddbussender.h \
    $$PWD/../../src/settings/dsettings.h \
    $$PWD/../../src/settings/dsettingsgroup.h \
    $$PWD/../../src/settings/dsettingsoption.h \
    $$PWD/../../src/dsysinfo.h \
    $$PWD/../../src/base/dsingleton.h \
    ut_dutil.h \
    ut_singleton.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../src/release/ -ldtkcore -lgtest
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../src/debug/ -ldtkcore -lgtest
else:unix: LIBS += -L$$OUT_PWD/../../src/ -ldtkcore -lgtest

INCLUDEPATH += \
    $$PWD/../../src \
    $$PWD/../../src/log
DEPENDPATH += $$PWD/../../src
QMAKE_RPATHDIR += $$PWD/../../src

RESOURCES += \
    data.qrc
