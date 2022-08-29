cmake_minimum_required (VERSION 3.10)

option(DVERSION "define project version" "5.5.22")
if(DVERSION)
  project (DtkCore
    VERSION ${DVERSION}
    DESCRIPTION "DTK Core module"
    HOMEPAGE_URL "https://github.com/linuxdeepin/dtkcore"
    LANGUAGES CXX C
  )
else()
  project (DtkCore
    #VERSION "${DTK_REPO_MODULE_VERSION}"
    VERSION "5.5.23"
    DESCRIPTION "DTK Core module"
    HOMEPAGE_URL "https://github.com/linuxdeepin/dtkcore"
    LANGUAGES CXX C
  )
endif()
message(STATUS ${PROJECT_VERSION})
include(GNUInstallDirs)
set (BUILD_EXAMPLES ON CACHE BOOL "Build examples")
option(BUILD_VERSION "buildversion" "0")
if(NOT BUILD_VERSION)
  set(BUILD_VERSION "0")
endif()

find_package (Qt5 CONFIG REQUIRED COMPONENTS DBus Xml)
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

configure_file(misc/DtkConfig.cmake.in DtkCoreConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DtkCoreConfig.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DtkCore")

#set(LINUXNAME "debian")
option(LINUXNAME "linuxname" OFF)
set(SPECPATH "qt5/mkspecs/")
if(LINUXNAME)
  if (${LINUXNAME} STREQUAL "archlinux")
    set(SPECPATH "qt/mkspecs/")
  endif()
else()
  set(LINUXNAME "debian")
endif()

message(STATUS ${LINUXNAME})
configure_file(misc/dtkcore.pc.in dtkcore.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dtkcore.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

configure_file(misc/qt_lib_dtkcore.pri.in qt_lib_dtkcore.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkcore.pri DESTINATION "${CMAKE_INSTALL_LIBDIR}/${SPECPATH}/modules")
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
