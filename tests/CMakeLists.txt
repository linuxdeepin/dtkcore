set(BIN_NAME "ut-${LIB_NAME}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# dbus
set(CMAKE_CXX_FLAGS "-fno-access-control")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
add_definitions(-DPREFIX="${CMAKE_INSTALL_PREFIX}")
add_definitions(-DOS_VERSION_TEST_FILE="/tmp/etc/os-version")
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
if(LINUX)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
  find_package(PkgConfig REQUIRED)
  if("${QT_VERSION_MAJOR}" STREQUAL "5")
    pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)
  endif()
  if(BUILD_WITH_SYSTEMD)
    pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
  endif()
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
find_package(GTest REQUIRED)

# start base
include(../src/base/base.cmake)

# end base

# start dci
include(../src/dci/dci.cmake)
#end dci

#start filesystem
include(../src/filesystem/filesystem.cmake)
#end filesystem
# start log
include(../src/log/log.cmake)
#end log
# start settings
include(../src/settings/settings.cmake)
#message(${settings_SRC})
#end settings

#start utils

# TODO match linux and others
include(../src/util/util.cmake)
#end utils

#GLOB
include(../src/glob.cmake)

#endglob

# for test.so
set(TEST_SO_NAME vtabletest${DTK_VERSION_MAJOR})
add_subdirectory(./testso)

# test
file(GLOB TEST_HEADER
  ut_.*h
)
file(GLOB TEST_SOURCE
  *.cpp
)

file(GLOB FackDBus
  "./fakedbus/*.h"
  "./fakedbus/*.cpp"
)

if("${QT_VERSION_MAJOR}" STREQUAL "6")
  list(REMOVE_ITEM TEST_SOURCE "${CMAKE_CURRENT_LIST_DIR}/ut_gsettingsbackend.cpp")
endif()

set(test_SRC
  ${TEST_HEADER}
  ${TEST_SOURCE}
  ${FackDBus}
)

# end test
if(LINUX)
  add_executable(${BIN_NAME}
    ${base_SRCS}
    ${dci_SRCS}
    ${filesystem_SRCS}
    ${log_SRCS}
    ${settings_SRC}
    ${utils_SRC}
    ${glob_SRC}
    ${test_SRC}
    ./data.qrc
  )
  add_dependencies(${BIN_NAME} ${TEST_SO_NAME})
  target_link_libraries(
    ${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Test
    ${GTEST_LIBRARIES}
    -lpthread
    -lm
    -lgcov
    -l${TEST_SO_NAME}
    -ldl
  )

  if(${QT_VERSION_MAJOR} STREQUAL "5")
      target_link_libraries(
        ${BIN_NAME} PRIVATE
        PkgConfig::QGSettings
      )
  endif()

  if(BUILD_WITH_SYSTEMD)
    target_link_libraries(
      ${BIN_NAME} PRIVATE
      PkgConfig::Systemd
    )
  endif()

  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
  endif()
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
  endif()
  target_link_directories(${BIN_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/testso/)
  target_include_directories(${BIN_NAME} PUBLIC
    ../include/util/
    ../include/dci/
    ../include/log/
    ../include/base/
    ../include/base/private/
    ../include/global/
    ../include/DtkCore/
    ../include/settings/
    ../include/filesystem/
    ../include/
    ../src/filesystem/private/
    ./testso/
  )
else()
  add_executable(${BIN_NAME}
    ${base_SRCS}
    ${dci_SRCS}
    ${filesystem_SRCS}
    ${log_SRCS}
    ${settings_SRC}
    ${utils_SRC}
    ${glob_SRC}
    ${test_SRC}
    ./data.qrc
  )
  add_dependencies(${BIN_NAME} ${TEST_SO_NAME})
  target_link_libraries(
    ${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Test
    ${GTEST_LIBRARIES}
    -lpthread
    -lm
    -lgcov
    -l${TEST_SO_NAME}
  )
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
  endif()
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
  endif()
  target_link_directories(${BIN_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/testso/)
  target_include_directories( ${BIN_NAME} PUBLIC
    ../include/util/
    ../include/dci/
    ../include/log/
    ../include/base/
    ../include/base/private/
    ../include/global/
    ../include/DtkCore/
    ../include/settings/
    ../include/filesystem/
    ../include/
    ./testso/
  )
endif()

add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
