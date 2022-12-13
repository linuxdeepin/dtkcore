set(BINNAME textcodec)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(${BINNAME}
    main.cpp
)

target_link_libraries(
    ${BINNAME} PRIVATE
    Qt5::Core
    dtkcore
)

target_include_directories(${BINNAME} PUBLIC
    ../../include/util/
    ../../include/
)
