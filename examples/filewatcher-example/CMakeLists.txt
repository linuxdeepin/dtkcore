set(BINNAME filewatcher)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(${BINNAME}
    main.cpp
)

target_link_libraries(
    ${BINNAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    dtkcore
)

target_include_directories(${BINNAME} PUBLIC
    ../../include/filesystem/
    ../../include/
)
