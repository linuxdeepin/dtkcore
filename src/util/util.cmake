if(LINUX)
  set(UTILS_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/dtimeunitformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dabstractunitformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/ddisksizeformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/ddbussender.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/drecentmanager.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dnotifysender.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dpinyin.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dexportedinterface.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dvtablehook.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dthreadutils.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dtimedloop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dfileservices_linux.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dexportedinterface.cpp
  )
else()
  set(UTILS_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/dtimeunitformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dabstractunitformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/ddisksizeformatter.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/ddbussender.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/drecentmanager.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dnotifysender.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dpinyin.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dexportedinterface.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dvtablehook.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dthreadutils.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/dtimedloop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dfileservices_dummy.cpp
  	${CMAKE_CURRENT_LIST_DIR}/dexportedinterface.cpp
  )
endif()
file(GLOB UTILS_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../../include/util/*
)
set(utils_SRC 
  ${UTILS_HEADER}
  ${UTILS_SOURCE}
  ${CMAKE_CURRENT_LIST_DIR}/util.qrc
)
