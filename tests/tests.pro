TEMPLATE = app
QT += core dbus xml testlib concurrent
CONFIG += thread c++11 link_pkgconfig
CONFIG -= app_bundle

QMAKE_LFLAGS += -Wl,--export-dynamic

CONFIG(debug, debug|release) {
QMAKE_CXXFLAGS += -g -Wall -fsanitize=address -fsanitize-recover=address -O2
QMAKE_LFLAGS += -g -Wall -fsanitize=address -fsanitize-recover=address -O2
QMAKE_CXX += -g -fsanitize=address -fsanitize-recover=address -O2
}

load(dtk_testcase)

# 指定moc文件生成目录和src一样
MOC_DIR=$$OUT_PWD/../src

# 使用 tmp 目录下的 os-version
DEFINES += OS_VERSION_TEST_FILE=\\\"/tmp/etc/os-version\\\"

DEPENDPATH += $$PWD/../src

unix: {
QMAKE_RPATHDIR += $$OUT_PWD/../src
LIBS += -L$$OUT_PWD/../src/ -ldtkcore -lgtest
# for dlsym
LIBS += -ldl
# TODO: vtabhook release test failed
QMAKE_CXXFLAGS_RELEASE -= -O2
}

INCLUDEPATH += \
    $$PWD/../src \
    $$PWD/../src/base \
    $$PWD/../src/base/private \
    $$PWD/../src/filesystem \
    $$PWD/../src/log \
    $$PWD/../src/settings \
    $$PWD/../src/util

include($$PWD/../src/base/base.pri)
include($$PWD/../src/filesystem/filesystem.pri)
include($$PWD/../src/log/log.pri)
include($$PWD/../src/settings/settings.pri)
include($$PWD/../src/util/util.pri)

HEADERS += $$PWD/ut_*.h \
    $$PWD/../src/dtkcore_global.h \
    $$PWD/../src/dsysinfo.h \
    $$PWD/../src/dsecurestring.h \
    $$PWD/../src/ddesktopentry.h

SOURCES += $$PWD/*.cpp \
    $$PWD/../src/dsysinfo.cpp \
    $$PWD/../src/dsecurestring.cpp \
    $$PWD/../src/ddesktopentry.cpp

RESOURCES += data.qrc
