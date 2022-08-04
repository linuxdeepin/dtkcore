set(BINNAME dasync)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(${BINNAME}
  main.cpp
)

target_link_libraries(
  ${BINNAME} PRIVATE 
  Qt5::Core 
  Qt5::Widgets
  dtkcore
  -lpthread
)
target_include_directories(${BINNAME} PUBLIC
  ../../include/global/
  ../../include/util/
  ../../include/
)
