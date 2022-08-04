file(GLOB LOG_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../../include/log/*
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
set(log_SRCS 
  ${LOG_HEADER}
  ${LOG_SOURCE}
)
