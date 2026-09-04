file(GLOB LOG_HEADER
  ${CMAKE_CURRENT_LIST_DIR}/../../include/log/LogManager.h
)
set(LOG_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/LogManager.cpp
  ${CMAKE_CURRENT_LIST_DIR}/dconfig_org_deepin_dtk_preference.hpp
)

set(log_SRCS
${LOG_HEADER}
${LOG_SOURCE}
)

