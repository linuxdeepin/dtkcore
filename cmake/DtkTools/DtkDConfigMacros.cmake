# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: LGPL-3.0-or-later

include(MacroAddFileDependencies)
include(CMakeParseArguments)

# Define the helper function
function(dtk_add_config_to_cpp OUTPUT_VAR JSON_FILE)
    if(NOT EXISTS ${JSON_FILE})
        message(FATAL_ERROR "JSON file ${JSON_FILE} does not exist.")
    endif()

    cmake_parse_arguments(
        "arg"
        ""
        "OUTPUT_FILE_NAME;CLASS_NAME"
        ""
        ${ARGN}
    )

    # Generate the output header file name
    get_filename_component(FILE_NAME_WLE ${JSON_FILE} NAME_WLE)
    if(DEFINED arg_OUTPUT_FILE_NAME)
        set(OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${arg_OUTPUT_FILE_NAME}")
    else()
        set(OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/dconfig_${FILE_NAME_WLE}.hpp")
    endif()

    # Check if CLASS_NAME is set
    if(DEFINED arg_CLASS_NAME)
        set(CLASS_NAME_ARG -c ${arg_CLASS_NAME})
    else()
        set(CLASS_NAME_ARG "")
    endif()

    # Add a custom command to run dconfig2cpp
    add_custom_command(
        OUTPUT ${OUTPUT_HEADER}
        COMMAND ${DTK_DCONFIG2CPP} -o ${OUTPUT_HEADER} ${CLASS_NAME_ARG} ${JSON_FILE}
        DEPENDS ${JSON_FILE} ${DTK_XML2CPP}
        COMMENT "Generating ${OUTPUT_HEADER} from ${JSON_FILE}"
        VERBATIM
    )

    # Add the generated header to the specified output variable
    set(${OUTPUT_VAR} ${${OUTPUT_VAR}} ${OUTPUT_HEADER} PARENT_SCOPE)
endfunction()
