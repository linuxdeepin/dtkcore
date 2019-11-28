QT -= gui
QT += dbus
QT += xml
CONFIG += link_pkgconfig
TARGET = dtkcore

INCLUDEPATH += $$PWD
HEADERS += $$PWD/dtkcore_global.h \
    dsysinfo.h \
    dsecurestring.h \
    ddesktopentry.h

SOURCES += \
    dsysinfo.cpp \
    dsecurestring.cpp \
    ddesktopentry.cpp

include($$PWD/base/base.pri)
include($$PWD/util/util.pri)
include($$PWD/log/log.pri)
include($$PWD/filesystem/filesystem.pri)
include($$PWD/settings/settings.pri)

DTK_MODULE_NAME = $$TARGET
include(dtk_build.prf)

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
include(dtk_cmake.prf)

#qt module
include(dtk_module.prf)

prf.files+= $$PWD/*.prf ../dtk_build_config.prf

isEmpty(MKSPECS_INSTALL_DIR) {
    MKSPECS_INSTALL_DIR=$$[QT_INSTALL_ARCHDATA]/mkspecs
}
prf.path = $${MKSPECS_INSTALL_DIR}/features

!linux {
    prf.files-=$$PWD/dtk_qmake.prf
}

gsettings.files += $$PWD/com.deepin.dtk.gschema.xml
gsettings.path = $${PREFIX}/share/glib-2.0/schemas

INSTALLS += prf gsettings
