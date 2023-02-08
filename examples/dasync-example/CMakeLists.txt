set(BINNAME dasync)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(${BINNAME}
  main.cpp
)

target_link_libraries(
  ${BINNAME} PRIVATE 
  Qt${QT_VERSION_MAJOR}::Core 
  Qt${QT_VERSION_MAJOR}::Widgets
  dtkcore
  -lpthread
)
target_include_directories(${BINNAME} PUBLIC
  ../../include/global/
  ../../include/util/
  ../../include/
)
