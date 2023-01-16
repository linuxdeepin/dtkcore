# SPDX-FileCopyrightText: 2023 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

file(GLOB_RECURSE TESTSO "*.cpp" "*.h")

add_library(vtabletest SHARED ${TESTSO})

target_link_libraries(vtabletest PUBLIC Qt5::Core)
