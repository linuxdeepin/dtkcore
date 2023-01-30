set(BINNAME dlog)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt5 REQUIRED COMPONENTS DBus)

add_executable(${BINNAME}
  main.cpp
)

target_link_libraries(
  ${BINNAME} PRIVATE
  Qt5::DBus
  dtkcore
)
target_include_directories(${BINNAME} PUBLIC
  ../../include/
)
