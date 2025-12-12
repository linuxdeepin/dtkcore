# 方便 test 脚本中调用
set(BIN_NAME "ut-DtkCore")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# dbus
set(CMAKE_CXX_FLAGS "-fno-access-control")
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

if(LINUX)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
    find_package(PkgConfig REQUIRED)
    if("${QT_VERSION_MAJOR}" STREQUAL "5")
        pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)
    elseif(${QT_VERSION_MAJOR} EQUAL 6)
        if(${Qt6Core_VERSION} VERSION_GREATER_EQUAL 6.10.0)
            find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS CorePrivate)
        endif()
    else()
        message(FATAL_ERROR "Unsupported Qt version: ${QT_VERSION_MAJOR}")
    endif()
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
find_package(GTest REQUIRED)

include(GoogleTest)

# for test.so
set(TEST_SO_NAME vtabletest${DTK_NAME_SUFFIX})
add_subdirectory(./testso)

# test
file(GLOB TEST_HEADER ut_.*h)
file(GLOB TEST_SOURCE *.cpp)

file(GLOB FackDBus
    "./fakedbus/*.h"
    "./fakedbus/*.cpp"
)

if(NOT DTK5)
    list(REMOVE_ITEM TEST_SOURCE "${CMAKE_CURRENT_LIST_DIR}/ut_gsettingsbackend.cpp")
    list(REMOVE_ITEM TEST_SOURCE "${CMAKE_CURRENT_LIST_DIR}/ut_dasync.cpp")
endif()

set(test_SRC
    ${TEST_HEADER}
    ${TEST_SOURCE}
    ${FackDBus}
)
# end test

add_executable(${BIN_NAME}
    ${test_SRC}
    ./data.qrc
)

target_compile_definitions(${BIN_NAME} PUBLIC
    PREFIX="${DSG_PREFIX_PATH}"
    DSYSINFO_PREFIX="${DSYSINFO_PREFIX}"
)

add_dependencies(${BIN_NAME} ${TEST_SO_NAME})
target_link_libraries(
    ${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Test
    ${GTEST_LIBRARIES}
    ${LIB_NAME}
    -l${TEST_SO_NAME}
)

if(LINUX)
    if(${QT_VERSION_MAJOR} STREQUAL "5")
          target_link_libraries(
              ${BIN_NAME} PRIVATE
              PkgConfig::QGSettings
          )
    endif()

    target_link_libraries(
        ${BIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::DBus
        -lpthread
        -lm
        -lgcov
        -ldl
    )
endif() #end LINUX

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

gtest_discover_tests(${BIN_NAME})
