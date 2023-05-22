cmake_minimum_required (VERSION 3.10)

set (DVERSION "5.6.8" CACHE STRING "define project version")

project (DtkCore
  VERSION ${DVERSION}
  DESCRIPTION "DTK Core module"
  HOMEPAGE_URL "https://github.com/linuxdeepin/dtkcore"
  LANGUAGES CXX C
)
message(STATUS ${PROJECT_VERSION})

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(QT_VERSION_MAJOR "5" CACHE STRING "Qt Version")
message("Current Qt Version: ${QT_VERSION_MAJOR}")

set (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/dtk${PROJECT_VERSION_MAJOR}/DCore")
set (TOOL_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/dtk${PROJECT_VERSION_MAJOR}/DCore/bin")
set (LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
set (MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "Install dir for qt pri files")
set (FEATURES_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/features" CACHE STRING "Install dir for qt prf files")
set (CONFIG_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/DtkCore" CACHE STRING "Install dir for cmake config files")

set (BUILD_EXAMPLES ON CACHE BOOL "Build examples")
set (BUILD_VERSION "0" CACHE STRING "buildversion")

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()
set (BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")

set (BUILD_WITH_SYSTEMD OFF CACHE BOOL "Build with systemd")

# CXX FILAGS
if("${QT_VERSION_MAJOR}" STREQUAL "5")
  set(CMAKE_CXX_STANDARD 11)
else()
  set(CMAKE_CXX_STANDARD 17)
endif()
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -pie")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
  endif ()
  string(REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif()

if (BUILD_DOCS)
  add_subdirectory(docs)
endif ()

add_subdirectory(src)
if(BUILD_TESTING)
  message("==================================")
  message("       Now Testing is enabled     ")
  message("==================================")
  enable_testing()
  add_subdirectory(tests)
endif()
if(BUILD_EXAMPLES)
  message("===================================")
  message("You can build and run examples now ")
  message("===================================")
  add_subdirectory(examples)
endif()
add_subdirectory(tools)
install(FILES cmake/DtkCMake/DtkCMakeConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkCMake")
install(FILES cmake/DtkTools/DtkToolsConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkTools")
install(FILES cmake/DtkTools/DtkSettingsToolsMacros.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkTools")
install(FILES cmake/DtkDConfig/DtkDConfigConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkDConfig")

configure_package_config_file(misc/DtkCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfig.cmake
    INSTALL_DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
    PATH_VARS TOOL_INSTALL_DIR)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfigVersion.cmake"
    VERSION ${DVERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfig.cmake DESTINATION ${CONFIG_CMAKE_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfigVersion.cmake DESTINATION ${CONFIG_CMAKE_INSTALL_DIR})

configure_file(misc/dtkcore.pc.in dtkcore.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dtkcore.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

configure_file(misc/qt_lib_dtkcore.pri.in qt_lib_dtkcore.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkcore.pri DESTINATION "${MKSPECS_INSTALL_DIR}")
install(FILES misc/dtk_install_dconfig.prf DESTINATION ${FEATURES_INSTALL_DIR})
set(CONFIGNAME include/global/dtkcore_config.h)
file(WRITE ${CONFIGNAME}
  "// it is auto make config\n"
  "#define DTK_VERSION_MAJOR ${PROJECT_VERSION_MAJOR}\n"
  "#define DTK_VERSION_MINOR ${PROJECT_VERSION_MINOR}\n"
  "#define DTK_VERSION_PATCH ${PROJECT_VERSION_PATCH}\n"
  "#define DTK_VERSION_BUILD ${BUILD_VERSION}\n"
  "#define DTK_VERSION_STR \"${PROJECT_VERSION}\"\n"
  "\n"
)
file(GLOB CONFIGSOURCE include/DtkCore/*)

foreach(FILENAME ${CONFIGSOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${CONFIGNAME} "#define DTKCORE_CLASS_${thefile}\n")
endforeach()
