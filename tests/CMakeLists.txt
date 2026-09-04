# 方便 test 脚本中调用
set(BIN_NAME "ut-DtkCore")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# dbus
set(CMAKE_CXX_FLAGS "-fno-access-control")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

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

# ---------------------------------------------------------------------------
# DSysInfo mock infrastructure
# ---------------------------------------------------------------------------
# The shared library (compiled in src/ before tests/) always has
# DSYSINFO_PREFIX="" so inTest() is false and singleton caches never clear.
# Two complementary mechanisms solve this:
#
# 1. OBJECT library: compile dsysinfo.cpp with DSYSINFO_PREFIX="/tmp" so
#    inTest()==true, paths point to /tmp/etc/, and caches clear on re-read.
#    Linked BEFORE the shared library so the test binary resolves DSysInfo
#    symbols from this copy (link-time stub, not compile-time macro in the
#    shared library).
#
# 2. LD_PRELOAD stub (dsysinfo_stub.cpp): redirects /etc/ -> /tmp/etc/ for
#    any reads originating from the shared library's internal code.
#
# Test-mode prefixes (also kept for the test binary's own compilation):
set(DSYSINFO_PREFIX "/tmp" CACHE STRING "PREFIX of DSysInfo" FORCE)
set(DSG_PREFIX_PATH "/tmp" CACHE STRING "PREFIX of DSG_DATA_DIRS" FORCE)

# OBJECT library — test-mode copy of dsysinfo.cpp
add_library(dsysinfo_test_obj OBJECT ../src/dsysinfo.cpp)
target_compile_definitions(dsysinfo_test_obj PRIVATE
    DSYSINFO_PREFIX="${DSYSINFO_PREFIX}"
    PREFIX="${DSG_PREFIX_PATH}"
)
target_include_directories(dsysinfo_test_obj PRIVATE
    ../include/global/
    ../include/
    ${PROJECT_BINARY_DIR}  # generated dtkcore_config.h lives at the build root
)
target_link_libraries(dsysinfo_test_obj PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

# Instrument the OBJECT library so dsysinfo.cpp gets coverage data.
# dtk_setup_code_coverage only applies to the shared library; the OBJECT
# library needs its own flags.
if(EnableCov OR BUILD_TESTING)
    target_compile_options(dsysinfo_test_obj PRIVATE -fprofile-arcs -ftest-coverage)
endif()

# LD_PRELOAD stub shared library
add_library(dsysinfo_stub SHARED dsysinfo_stub.cpp)
target_link_libraries(dsysinfo_stub PRIVATE dl)

# ---------------------------------------------------------------------------

# test
file(GLOB TEST_HEADER ut_.*h)
file(GLOB TEST_SOURCE *.cpp)

# Exclude the LD_PRELOAD stub from the test binary sources
list(REMOVE_ITEM TEST_SOURCE "${CMAKE_CURRENT_LIST_DIR}/dsysinfo_stub.cpp")

file(GLOB FackDBus
    "./fakedbus/*.h"
    "./fakedbus/*.cpp"
)

if(NOT DTK5)
    list(REMOVE_ITEM TEST_SOURCE "${CMAKE_CURRENT_LIST_DIR}/ut_gsettingsbackend.cpp")
    list(REMOVE_ITEM TEST_SOURCE "${CMAKE_CURRENT_LIST_DIR}/ut_dasync.cpp")
    list(REMOVE_ITEM TEST_SOURCE "${CMAKE_CURRENT_LIST_DIR}/ut_dtimedloop.cpp")
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

# Instrument the test binary so that source compiled into the test
# executable (e.g. LogManager.cpp #included by ut_dlog.cpp, and inline
# functions from headers) generates coverage data. Without this, lcov
# reports 0% for LogManager.cpp and low coverage for header inline code.
if(EnableCov OR BUILD_TESTING)
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

add_dependencies(${BIN_NAME} ${TEST_SO_NAME})

# Link the OBJECT library FIRST so its DSysInfo symbols take precedence
# over the shared library's copies (which have DSYSINFO_PREFIX="").
target_link_libraries(
    ${BIN_NAME} PRIVATE
    dsysinfo_test_obj
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
    ../src/log/
    ../src/dci/private/
    ../src/util/
    ../src/filesystem/
    ./testso/
)

# Set RPATH to find libvtabletest*.so in the build directory
set_target_properties(${BIN_NAME} PROPERTIES
    SKIP_BUILD_RPATH FALSE
)

# Register tests with LD_PRELOAD environment so the stub redirects /etc/
# reads for any shared-library internal code paths.
gtest_discover_tests(${BIN_NAME}
    PROPERTIES
    ENVIRONMENT "LD_PRELOAD=$<TARGET_FILE:dsysinfo_stub>"
)

# ---- run-test: execute all tests and generate a coverage report ----
find_program(LCOV_BIN lcov)
find_program(GENHTML_BIN genhtml)
set(_run_test_coverage "")
if(LCOV_BIN AND GENHTML_BIN)
    set(_run_test_coverage
        COMMAND ${LCOV_BIN} --capture --directory ${CMAKE_BINARY_DIR}
                --rc lcov_branch_coverage=1 --output-file ${CMAKE_BINARY_DIR}/coverage.info
        COMMAND ${LCOV_BIN} --remove ${CMAKE_BINARY_DIR}/coverage.info
                --rc lcov_branch_coverage=1
                '*/tests/*' '*/moc_*' '*/qrc_*' '*/ui_*' '*/qml_*' '*/_autogen/*'
                '*_json.h' '/usr/*'
                --output-file ${CMAKE_BINARY_DIR}/coverage_filtered.info
        COMMAND ${GENHTML_BIN} ${CMAKE_BINARY_DIR}/coverage_filtered.info
                --branch-coverage --output-directory ${CMAKE_BINARY_DIR}/coverage_report
    )
endif()
add_custom_target(run-test
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    ${_run_test_coverage}
    DEPENDS ${BIN_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Run all tests and generate coverage report"
    VERBATIM
)
