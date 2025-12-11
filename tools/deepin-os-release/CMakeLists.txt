set(TARGET_NAME deepin-os-release)
set(BIN_NAME ${TARGET_NAME}${DTK_VERSION_MAJOR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
if(${QT_VERSION_MAJOR} EQUAL 6)
    if(${Qt6Core_VERSION} VERSION_GREATER_EQUAL 6.10.0)
      find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS CorePrivate)
    endif()
endif()

add_definitions(-DDTK_NO_PROJECT)
# start dci
set(dci_SRCS
  ../../include/global/dsysinfo.h
  ../../include/global/ddesktopentry.h
  ../../src/dsysinfo.cpp
  ../../src/ddesktopentry.cpp
)
add_executable(${BIN_NAME}
  ${dci_SRCS}
  main.cpp
)

target_compile_definitions(${BIN_NAME} PUBLIC
    DSYSINFO_PREFIX="${DSYSINFO_PREFIX}")

target_link_libraries(${BIN_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::CorePrivate
)
target_include_directories(${BIN_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/dci>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/DtkCore>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/base>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/global>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
)
set_target_properties(${BIN_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")

#end dci
