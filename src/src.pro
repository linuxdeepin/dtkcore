QT -= gui
QT += dbus
CONFIG += link_pkgconfig

TARGET = dtkcore

DEFINES += LIBDTKCORE_LIBRARY

INCLUDEPATH += $$PWD $$PWD/base

HEADERS += \
    $$PWD/dtkcore_global.h

include($$PWD/lib.pri)
include($$PWD/base/base.pri)
include($$PWD/util/util.pri)
include($$PWD/log/log.pri)
include($$PWD/filesystem/filesystem.pri)
include($$PWD/settings/settings.pri)

# ----------------------------------------------
# install config

includes.files += $$PWD/*.h $$PWD/dtkcore_config.h $$PWD/DtkCore
includes.path = $${DTK_INCLUDEPATH}/DCore

# CMake configure
INC_DIR = $$replace(includes.path, "/", "\/")
CMD = sed -i -E \'s/INCLUDE_INSTALLED_DIR \".*\"\\)$/INCLUDE_INSTALLED_DIR \"$${INC_DIR}\"\\)/\' ../cmake/DtkCore/DtkCoreConfig.cmake
system($$CMD)

cmake_config.path = $$LIB_INSTALL_DIR
cmake_config.files = $$PWD/../cmake

INSTALLS += includes target cmake_config

QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

QMAKE_PKGCONFIG_NAME = DTK_CORE
QMAKE_PKGCONFIG_DESCRIPTION = Deepin Tool Kit Core Header Files
QMAKE_PKGCONFIG_INCDIR = $$includes.path

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
