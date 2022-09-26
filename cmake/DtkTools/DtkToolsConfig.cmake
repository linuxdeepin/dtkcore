find_package(DtkCore REQUIRED)

set (DTK_SETTINGS_TOOLS_EXECUTABLE ${DtkCore_TOOL_DIRS}/dtk-settings)

if (EXISTS ${DTK_SETTINGS_TOOLS_EXECUTABLE})
    set(DTK_SETTINGS_TOOLS_FOUND TRUE)
endif ()

include("${CMAKE_CURRENT_LIST_DIR}/DtkSettingsToolsMacros.cmake")