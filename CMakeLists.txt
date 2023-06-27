cmake_minimum_required (VERSION 3.13)

set (DTK_VERSION "5.6.12" CACHE STRING "define project version")
project (DtkCore
  VERSION ${DTK_VERSION}
  DESCRIPTION "DTK Core module"
  HOMEPAGE_URL "https://github.com/linuxdeepin/dtkcore"
  LANGUAGES CXX C
)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if("${PROJECT_VERSION_MAJOR}" STREQUAL "5")
    set(QT_VERSION_MAJOR "5")
elseif("${PROJECT_VERSION_MAJOR}" STREQUAL "6")
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
else()
    message(SEND_ERROR "not support Prject Version ${PROJECT_VERSION}.")
endif()

if(APPLE)
    # Force use qt6 on macOS
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")

    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

include(dtkcore.cmake)
