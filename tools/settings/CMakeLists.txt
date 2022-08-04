set(BIN_NAME dtk-settings)

find_package(Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 REQUIRED COMPONENTS Xml)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
add_executable(${BIN_NAME}
  main.cpp
)
target_link_libraries(
  ${BIN_NAME} PRIVATE
  Qt5::Core
  Qt5::Xml
  dtkcore
  ${QGSettings_LIBRARIES}
)
target_include_directories( ${BIN_NAME} PUBLIC
  ${QGSettings_INCLUDE_DIRS}
  ../../include/util/
  ../../include/dci/
  ../../include/log/
  ../../include/base/
  ../../include/global/
  ../../include/DtkCore/
  ../../include/settings/
  ../../include/filesystem/
  ../../include/
)
install(TARGETS ${BIN_NAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}/libdtk-${PROJECT_VERSION}/DCore/bin/")
