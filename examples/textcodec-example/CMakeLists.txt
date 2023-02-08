set(BINNAME textcodec)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(${BINNAME}
    main.cpp
)

target_link_libraries(
    ${BINNAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    dtkcore
)

if("${QT_VERSION_MAJOR}" STREQUAL "6")
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat)
    target_link_libraries(
    ${BINNAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core5Compat
)
endif()

target_include_directories(${BINNAME} PUBLIC
    ../../include/util/
    ../../include/
)
