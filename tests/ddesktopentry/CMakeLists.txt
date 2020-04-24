# run test with verbose log: `ctest --verbose`

# QXdgDesktopEntryTest
add_executable (DDesktopEntryTest
    tst_ddesktopentrytest.cpp
)
add_test (NAME DDesktopEntryTest COMMAND DDesktopEntryTest )
target_link_libraries (DDesktopEntryTest ${DtkCore_LIBRARIES} Qt5::Test)
