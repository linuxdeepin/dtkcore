#cmake_minimum_required(VERSION 3.5)
set(LIBNAME dtkcore)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
set (DSG_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}" CACHE STRING "PREFIX of DSG_DATA_DIRS")
add_definitions(-DPREFIX="${DSG_PREFIX_PATH}")
find_package(Qt5 REQUIRED COMPONENTS Core)
if(LINUX)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
find_package(Qt5 REQUIRED COMPONENTS DBus)
endif()
find_package(Qt5 REQUIRED COMPONENTS Xml)

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
    ${LIBNAME} PRIVATE 
    Qt5::Core 
    Qt5::DBus
    Qt5::Xml
    ${QGSettings_LIBRARIES}
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
    ${LIBNAME} PRIVATE 
    Qt5::Core 
    Qt5::Xml
  )
endif()
set_target_properties(${LIBNAME} PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
)
target_include_directories( ${LIBNAME} PUBLIC
  ${QGSettings_INCLUDE_DIRS}
  ${Qt5Core_PRIVATE_INCLUDE_DIRS}
  ../include/util/
  ../include/dci/
  ../include/log/
  ../include/base/
  ../include/base/private
  ../include/global/
  ../include/DtkCore/
  ../include/settings/
  ../include/filesystem/
  ../include/
)
set(TOINSTALLBASE
  ../include/base/dobject.h
  ../include/base/dsingleton.h
  ../include/base/private/dobject_p.h
)
install(FILES ${TOINSTALLBASE} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore/")
install(DIRECTORY ../include/dci/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore/" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ../include/DtkCore/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore/" FILES_MATCHING PATTERN "*")
install(DIRECTORY ../include/filesystem/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore/" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ../include/global/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore/" FILES_MATCHING PATTERN "*.h")
file(GLOB TOINSTALLLOG 
  ../include/log/*.h
)
install(FILES ${TOINSTALLLOG} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore/")
file(GLOB TOINSTALLSETTINGS 
  ../include/settings/*.h
  ../include/settings/backend/*.h
)
install(FILES ${TOINSTALLSETTINGS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore/")
install(DIRECTORY ../include/util/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libdtk-${CMAKE_PROJECT_VERSION}/DCore/" FILES_MATCHING PATTERN "*.h")
install(TARGETS ${LIBNAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
