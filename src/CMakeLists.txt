#cmake_minimum_required(VERSION 3.5)
set(LIBNAME dtkcore)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set (DSG_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}" CACHE STRING "PREFIX of DSG_DATA_DIRS")
add_definitions(-DPREFIX="${DSG_PREFIX_PATH}")
add_definitions(-DLIBDTKCORE_LIBRARY)
find_package(Qt5 REQUIRED COMPONENTS Core)
if(LINUX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
find_package(Qt5 REQUIRED COMPONENTS DBus)
endif()
find_package(Qt5 REQUIRED COMPONENTS Xml)

# start text encoding
find_package(ICU REQUIRED COMPONENTS uc)
pkg_check_modules(uchardet REQUIRED uchardet)
# end text encoding

# start base 
include(base/base.cmake)
# end base
if(LINUX)
  include(dbus/dbus.cmake)
endif()
#message(${dbus_SRCS})
# end dbus

# start dci
include(dci/dci.cmake)
#end dci

#start filesystem
include(filesystem/filesystem.cmake)
#end filesystem
# start log 
include(log/log.cmake)
#end log
# start settings 
include(settings/settings.cmake)
#end settings

#start utils 
include(util/util.cmake)
#end utils

#GLOB
include(glob.cmake)
#endGLOG
if(LINUX)
  add_library(${LIBNAME} SHARED 
    ${dbus_SRCS}
    ${base_SRCS}
    ${dci_SRCS}
    ${filesystem_SRCS}
    ${log_SRCS}
    ${settings_SRC}
    ${utils_SRC}
    ${glob_SRC}
  )
  target_link_libraries(
    ${LIBNAME} PUBLIC 
    Qt5::Core 
    Qt5::DBus
    Qt5::Xml
    ${QGSettings_LIBRARIES}
    ICU::uc
    uchardet
  )
else()
  add_library(${LIBNAME} SHARED 
    ${base_SRCS}
    ${dci_SRCS}
    ${filesystem_SRCS}
    ${log_SRCS}
    ${settings_SRC}
    ${utils_SRC}
    ${glob_SRC}
  )
  target_link_libraries(
    ${LIBNAME} PUBLIC 
    Qt5::Core 
    Qt5::Xml
    ICU::uc
    uchardet
  )
endif()
set_target_properties(${LIBNAME} PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
  EXPORT_NAME Core
)
target_include_directories( ${LIBNAME} PUBLIC
  ${QGSettings_INCLUDE_DIRS}
  ${Qt5Core_PRIVATE_INCLUDE_DIRS}
  ${uchardet_INCLUDE_DIRS}
)
target_include_directories(${LIBNAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/util>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/dci>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/log>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/base>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/base/private>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/global>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/DtkCore>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/settings>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/filesystem>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_include_directories(${LIBNAME} INTERFACE
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_link_directories(${LIBNAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${LIBRARY_INSTALL_DIR}>
)

set(TOINSTALLBASE
  ../include/base/dobject.h
  ../include/base/dsingleton.h
  ../include/base/private/dobject_p.h
  ../include/base/derror.h
  ../include/base/dexpected.h
)
install(FILES ${TOINSTALLBASE} DESTINATION "${INCLUDE_INSTALL_DIR}")
install(DIRECTORY ../include/dci/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ../include/DtkCore/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*")
install(DIRECTORY ../include/filesystem/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ../include/global/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h")
file(GLOB TOINSTALLLOG 
  ../include/log/*.h
)
install(FILES ${TOINSTALLLOG} DESTINATION "${INCLUDE_INSTALL_DIR}")
file(GLOB TOINSTALLSETTINGS 
  ../include/settings/*.h
  ../include/settings/backend/*.h
)
install(FILES ${TOINSTALLSETTINGS} DESTINATION "${INCLUDE_INSTALL_DIR}")
install(DIRECTORY ../include/util/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h")
install(TARGETS ${LIBNAME} EXPORT DtkCoreTargets
 DESTINATION ${LIBRARY_INSTALL_DIR})

install(EXPORT DtkCoreTargets
  FILE DtkCoreTargets.cmake
  NAMESPACE Dtk::
  DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
)
