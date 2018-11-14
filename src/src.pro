QT -= gui
QT += dbus
QT += xml
CONFIG += link_pkgconfig
TARGET = dtkcore

include(dtk_build.prf)

INCLUDEPATH += $$PWD
HEADERS += $$PWD/dtkcore_global.h \
    dsysinfo.h

SOURCES += \
    dsysinfo.cpp

include($$PWD/base/base.pri)
include($$PWD/util/util.pri)
include($$PWD/log/log.pri)
include($$PWD/filesystem/filesystem.pri)
include($$PWD/settings/settings.pri)

# create DtkCore file
defineTest(containIncludeFiles) {
    header = $$absolute_path($$ARGS)
    header_dir = $$quote($$dirname(header))

    for (file, includes.files) {
        file_ap = $$absolute_path($$file)
        file_dir = $$quote($$dirname(file_ap))

        isEqual(file_dir, $$header_dir) {
            return(true)
        }
    }

    return(false)
}

defineTest(updateDtkCoreFile) {
    include_files = $$HEADERS
    file_content = $$quote($${LITERAL_HASH}ifndef DTK_CORE_MODULE_H)
    file_content += $$quote($${LITERAL_HASH}define DTK_CORE_MODULE_H)

    for(header, include_files) {
        containIncludeFiles($$header) {
            file_content += $$quote($${LITERAL_HASH}include \"$$basename(header)\")
        }
    }

    file_content += $$quote($${LITERAL_HASH}endif)
    !write_file($$PWD/DtkCore, file_content):return(false)

    return(true)
}

!updateDtkCoreFile():warning(Cannot create "DtkCore" header file)

# create dtkwidget_config.h file
defineTest(updateDtkCoreConfigFile) {
    config_content += $$quote($${LITERAL_HASH}define DTK_VERSION_MAJOR $$VER_MAJ)
    config_content += $$quote($${LITERAL_HASH}define DTK_VERSION_MINOR $$VER_MIN)
    config_content += $$quote($${LITERAL_HASH}define DTK_VERSION_PATCH $$VER_PAT)
    config_content += $$quote($${LITERAL_HASH}define DTK_VERSION_BUILD $$VER_BUI)
    config_content += $$quote($${LITERAL_HASH}define DTK_VERSION_STR \"$$VERSION\")
    config_content += $$quote(//)

    for(file, includes.files) {
        file = $$quote($$basename(file))

        !isEqual(file, DtkCore):contains(file, D[A-Za-z0-9_]+) {
            config_content += $$quote($${LITERAL_HASH}define DTKCORE_CLASS_$$file)
        }
    }

    !write_file($$PWD/dtkcore_config.h, config_content):return(false)

    return(true)
}

!updateDtkCoreConfigFile():warning(Cannot create "dtkcore_config.h" header file)

# ----------------------------------------------
# install config
includes.files += \
    $$PWD/*.h \
    $$PWD/dtkcore_config.h \
    $$PWD/DtkCore \
    $$PWD/DSysInfo

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

prf.files+= $$PWD/*.prf
prf.path = $${QT_HOST_DATA}/mkspecs/features

linux {
    # dtk for qmake
    include(dtk_qmake.prf)

    deepin_os_release_tool.files=$$PWD/../bin/deepin-os-release
    deepin_os_release_tool.path=$$TOOL_INSTALL_DIR

    INSTALLS += deepin_os_release_tool
} else {
    prf.files-=$$PWD/dtk_qmake.prf
}

INSTALLS += prf
