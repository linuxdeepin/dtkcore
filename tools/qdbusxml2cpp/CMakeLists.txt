set(BIN_NAME qdbusxml2cpp-fix)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS DBus)

add_executable(${BIN_NAME}
  qdbusxml2cpp.cpp 
)

target_link_libraries(
  ${BIN_NAME} PRIVATE
  Qt5::DBus
  Qt5::Core 
)
target_include_directories(${BIN_NAME} PUBLIC
  ${Qt5DBus_PRIVATE_INCLUDE_DIRS}
)
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
