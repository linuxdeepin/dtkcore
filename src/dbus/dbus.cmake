set(dbus_SRCS)
set_source_files_properties(
  ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.xml 
  PROPERTIES 
  NO_NAMESPACE ON 
  CLASSNAME DSGConfig
)
qt5_add_dbus_interface(dbus_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.xml
  configmanager_interface
)
set_source_files_properties(
  ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.Manager.xml 
  PROPERTIES 
  NO_NAMESPACE ON 
  CLASSNAME DSGConfigManager
)
qt5_add_dbus_interface(dbus_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.Manager.xml
  manager_interface
)
