set(BIN_NAME dci)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
add_definitions(-DDTK_NO_PROJECT)
# start dci
include(../../src/dci/dci.cmake)
add_executable(${BIN_NAME}
  ${dci_SRCS}
  main.cpp
)
target_link_libraries(${BIN_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::CorePrivate
)
target_include_directories(${BIN_NAME} PUBLIC
  ../../include/
  ../../include/dci/
  ../../include/DtkCore/
  ../../include/base/
  ../../include/global/
)
#end dci

install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
