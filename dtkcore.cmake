set(LIB_NAME dtk${DTK_NAME_SUFFIX}core)
set(DtkCore Dtk${DTK_NAME_SUFFIX}Core)

macro(add_sub_dir dir)
    add_subdirectory(${dir} ${OUTPUT_DIR}/${dir})
endmacro()

message(STATUS "Compiling with DTK major version: ${DTK_VERSION_MAJOR}, Qt major version: ${QT_VERSION_MAJOR}")

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
set (LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
set (INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/dtk${DTK_VERSION_MAJOR}/DCore")
set (TOOL_INSTALL_DIR "${CMAKE_INSTALL_LIBEXECDIR}/dtk${DTK_VERSION_MAJOR}/DCore/bin")
set (MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "Install dir for qt pri files")
set (FEATURES_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/features" CACHE STRING "Install dir for qt prf files")
set (CONFIG_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${DtkCore}" CACHE STRING "Install dir for cmake config files")
set (DSG_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}" CACHE STRING "PREFIX of DSG_DATA_DIRS")
set (DSYSINFO_PREFIX "" CACHE STRING "PREFIX of DSysInfo")

set (BUILD_EXAMPLES ON CACHE BOOL "Build examples")
set (BUILD_VERSION "0" CACHE STRING "buildversion")

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

set (BUILD_WITH_SYSTEMD OFF CACHE BOOL "Build with systemd")
if (BUILD_WITH_SYSTEMD)
    add_definitions(-DBUILD_WITH_SYSTEMD)
endif()

set(CMAKE_CXX_STANDARD 17)

# CXX FILAGS
if("${QT_VERSION_MAJOR}" STREQUAL "5")
  set (BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")
else()
  # dtk6 not build doc
  set (BUILD_DOCS OFF CACHE BOOL "Generate doxygen-based documentation")
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
  string(REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

if (BUILD_DOCS)
  add_sub_dir(docs)
endif ()

add_sub_dir(src)
if(BUILD_TESTING)
  message("==================================")
  message("       Now Testing is enabled     ")
  message("==================================")
  enable_testing()
  add_sub_dir(tests)
endif()
if(BUILD_EXAMPLES)
  message("===================================")
  message("You can build and run examples now ")
  message("===================================")
  add_sub_dir(examples)
endif()
add_sub_dir(tools)

configure_package_config_file(cmake/DtkCMake/DtkCMakeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/DtkCMake/Dtk${DTK_NAME_SUFFIX}CMakeConfig.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}CMake"
    PATH_VARS TOOL_INSTALL_DIR)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/DtkCMake/Dtk${DTK_NAME_SUFFIX}CMakeConfig.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}CMake")

configure_package_config_file(cmake/DtkTools/DtkToolsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/DtkTools/Dtk${DTK_NAME_SUFFIX}ToolsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}Tools
    PATH_VARS TOOL_INSTALL_DIR)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/DtkTools/Dtk${DTK_NAME_SUFFIX}ToolsConfig.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}Tools")

install(FILES cmake/DtkTools/DtkSettingsToolsMacros.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}Tools"
    RENAME Dtk${DTK_NAME_SUFFIX}SettingsToolsMacros.cmake)

install(FILES cmake/DtkTools/DtkDBusMacros.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}Tools")

install(FILES ${CMAKE_SOURCE_DIR}/cmake/DtkTools/DtkDConfigMacros.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}Tools")

if (NOT DTK5)
    set(DCONFIG_DEPRECATED_FUNCS [=[
# deprecated since dtk6
function(dconfig_meta_files)
    dtk_add_config_meta_files(${ARGV})
endfunction()
function(dconfig_override_files)
    dtk_add_config_override_files(${ARGV})
endfunction()]=])
endif()

configure_package_config_file(cmake/DtkDConfig/DtkDConfigConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/DtkDConfig/Dtk${DTK_NAME_SUFFIX}DConfigConfig.cmake
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}DConfig"
    PATH_VARS TOOL_INSTALL_DIR)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/DtkDConfig/Dtk${DTK_NAME_SUFFIX}DConfigConfig.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_NAME_SUFFIX}DConfig")

configure_package_config_file(misc/DtkCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DtkCore}Config.cmake
    INSTALL_DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
    PATH_VARS TOOL_INSTALL_DIR)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${DtkCore}ConfigVersion.cmake"
    VERSION ${DTK_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DtkCore}Config.cmake DESTINATION ${CONFIG_CMAKE_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DtkCore}ConfigVersion.cmake DESTINATION ${CONFIG_CMAKE_INSTALL_DIR})

configure_file(misc/dtkcore.pc.in ${LIB_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

configure_file(misc/qt_lib_dtkcore.pri.in qt_lib_dtkcore.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkcore.pri DESTINATION "${MKSPECS_INSTALL_DIR}")
install(FILES misc/dtk_install_dconfig.prf DESTINATION ${FEATURES_INSTALL_DIR})
set(CONFIGNAME ${PROJECT_BINARY_DIR}/dtkcore_config.h)
file(WRITE ${CONFIGNAME}
  "// it is auto make config\n"
  "#define DTK_VERSION_MAJOR ${DTK_VERSION_MAJOR}\n"
  "#define DTK_VERSION_MINOR ${DTK_VERSION_MINOR}\n"
  "#define DTK_VERSION_PATCH ${DTK_VERSION_PATCH}\n"
  "#define DTK_VERSION_BUILD ${BUILD_VERSION}\n"
  "#define DTK_VERSION_STR \"${DTK_VERSION}\"\n"
  "\n"
)
file(GLOB CONFIGSOURCE include/DtkCore/*)

foreach(FILENAME ${CONFIGSOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${CONFIGNAME} "#define DTKCORE_CLASS_${thefile}\n")
endforeach()
install(FILES ${CONFIGNAME} DESTINATION "${INCLUDE_INSTALL_DIR}")
