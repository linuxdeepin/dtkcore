cmake_minimum_required (VERSION 3.10)

set (DVERSION "5.6.0.2" CACHE STRING "define project version")

project (DtkCore
  VERSION ${DVERSION}
  DESCRIPTION "DTK Core module"
  HOMEPAGE_URL "https://github.com/linuxdeepin/dtkcore"
  LANGUAGES CXX C
)
message(STATUS ${PROJECT_VERSION})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
set (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore")
set (TOOL_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/libdtk-${PROJECT_VERSION}/DCore/bin")
set (LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}")
set (MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/qt5/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")

set (BUILD_EXAMPLES ON CACHE BOOL "Build examples")
option(BUILD_VERSION "buildversion" "0")
if(NOT BUILD_VERSION)
  set(BUILD_VERSION "0")
endif()

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()
set (BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")

# CXX FILAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fsanitize=address -fno-omit-frame-pointer")
  set(BUILD_TESTING ON)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()

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
install(FILES cmake/DtkCMake/DtkCMakeConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkCMake/")
install(FILES cmake/DtkTools/DtkToolsConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkTools")
install(FILES cmake/DtkTools/DtkSettingsToolsMacros.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkTools")

configure_package_config_file(misc/DtkConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfig.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkCore"
    PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR TOOL_INSTALL_DIR)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfigVersion.cmake"
    VERSION ${DVERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkCore")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfigVersion.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkCore")

configure_file(misc/dtkcore.pc.in dtkcore.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dtkcore.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

configure_file(misc/qt_lib_dtkcore.pri.in qt_lib_dtkcore.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkcore.pri DESTINATION "${MKSPECS_INSTALL_DIR}")
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
