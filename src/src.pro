QT -= gui
QT += dbus
QT += xml
CONFIG += link_pkgconfig
TARGET = dtkcore

# 龙芯架构上没有默认添加PT_GNU_STACK-section,所以此处手动指定一下
contains(QMAKE_HOST.arch, mips.*): QMAKE_LFLAGS_SHLIB += "-Wl,-z,noexecstack"

QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden

INCLUDEPATH += $$PWD
HEADERS += $$PWD/dtkcore_global.h \
    dsysinfo.h \
    dsecurestring.h \
    ddesktopentry.h

SOURCES += \
    dsysinfo.cpp \
    dsecurestring.cpp \
    ddesktopentry.cpp \
    dtkcore_global.cpp

include($$PWD/base/base.pri)
include($$PWD/util/util.pri)
include($$PWD/log/log.pri)
include($$PWD/filesystem/filesystem.pri)
include($$PWD/settings/settings.pri)

DTK_MODULE_NAME = $$TARGET
load(dtk_build)

# ----------------------------------------------
# install config
includes.files += \
    $$PWD/*.h \
    $$PWD/dtkcore_config.h \
    $$PWD/DtkCores \
    $$PWD/DSysInfo \
    $$PWD/DSecureString \
    $$PWD/DDesktopEntry

INSTALLS += includes target

isEmpty(DTK_STATIC_LIB){
    DEFINES += LIBDTKCORE_LIBRARY
} else {
    DEFINES += DTK_STATIC_LIB
}

#cmake
load(dtk_cmake)

#qt module
load(dtk_module)

# 支持上游一包多依赖
load(dtk_multiversion)
# 5.5 5.6可通过重复调用此函数,来增加对更多版本的支持
dtkBuildMultiVersion(5.5)

# INSTALL变量增加多版本下的配置文件
load(dtk_install_multiversion)
