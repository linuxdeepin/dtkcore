file(GLOB LOG_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../../include/log/*.h
)
set(LOG_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/RollingFileAppender.cpp
  ${CMAKE_CURRENT_LIST_DIR}/Logger.cpp
  ${CMAKE_CURRENT_LIST_DIR}/FileAppender.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ConsoleAppender.cpp
  ${CMAKE_CURRENT_LIST_DIR}/AbstractStringAppender.cpp
  ${CMAKE_CURRENT_LIST_DIR}/AbstractAppender.cpp
  ${CMAKE_CURRENT_LIST_DIR}/LogManager.cpp
)

if(BUILD_WITH_SYSTEMD AND UNIX AND NOT APPLE)
  list(APPEND LOG_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/JournalAppender.cpp
  )
endif()

if(WIN32)
  set(log_SRCS
    ${LOG_HEADER}
    ${LOG_SOURCE}
    ${CMAKE_CURRENT_LIST_DIR}/OutputDebugAppender.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../include/log/win32/OutputDebugAppender.h
  )
else()
  set(log_SRCS
    ${LOG_HEADER}
    ${LOG_SOURCE}
  )
endif()
