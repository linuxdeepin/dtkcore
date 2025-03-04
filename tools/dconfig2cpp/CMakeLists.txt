set(TARGET_NAME dconfig2cpp)
set(BIN_NAME ${TARGET_NAME}${DTK_VERSION_MAJOR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(${BIN_NAME}
  main.cpp
)

target_link_libraries(
  ${BIN_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
)

set_target_properties(
  ${BIN_NAME} PROPERTIES
  OUTPUT_NAME ${TARGET_NAME}
  EXPORT_NAME DConfig2Cpp
)

install(
  TARGETS ${BIN_NAME}
  EXPORT Dtk${DTK_VERSION_MAJOR}ToolsTargets
  DESTINATION ${TOOL_INSTALL_DIR}
)
