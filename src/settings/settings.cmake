if(LINUX)
  file(GLOB SETTINGS_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backend/*.cpp
  )
  file(GLOB SETTINGS_HEADER 
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/*.h 
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/backend/*.h 
  )
else()
  file(GLOB SETTINGS_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backend/dsettingsdconfigbackend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/backend/qsettingbackend.cpp
  )
  file(GLOB SETTINGS_HEADER 
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/*.h 
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/backend/dsettingsdconfigbackend.h 
    ${CMAKE_CURRENT_LIST_DIR}/../../include/settings/backend/qsettingbackend.h
  )
endif()
set(settings_SRC
  ${SETTINGS_HEADER}
  ${SETTINGS_SOURCE}
)
