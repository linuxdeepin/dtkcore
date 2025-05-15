cmake_minimum_required (VERSION 3.13)

project (DtkCore
    VERSION 5.7.15
    DESCRIPTION "DTK Core module"
    HOMEPAGE_URL "https://github.com/linuxdeepin/dtkcore"
    LANGUAGES CXX C
)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Note that this option is solely for developer to quickly switch to the development of DTK6
# Please never ever ever use this option in packaging environment
set(DTK6_INITIAL_VALUE OFF)
if(${PROJECT_VERSION_MAJOR} STREQUAL "6")
    set(DTK6_INITIAL_VALUE ON)
endif()
option(DTK6 "Build DTK6 or not." ${DTK6_INITIAL_VALUE})
if(DTK6)
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
else()
    set(QT_VERSION_MAJOR "5")
endif()

include(dtkcore.cmake)
