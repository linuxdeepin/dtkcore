#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
set -ex

BUILD_DIR=`pwd`/../build/tests/
HTML_DIR=${BUILD_DIR}/html
XML_DIR=${BUILD_DIR}/report

export ASAN_OPTIONS="halt_on_error=0"

# back to project directroy
cd ..

osv_def="-DOS_VERSION_TEST_FILE=\"/tmp/etc/os-version\""
lsb_def="-DLSB_RELEASE_TEST_FILE=\"/tmp/etc/lsb-release\""
os_def="-DOS_RELEASE_TEST_FILE=\"/tmp/etc/os-release\""
dpv_def="-DDEEPIN_VERSION_TEST_FILE=\"/tmp/etc/deepin-version\""

cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_EXAMPLE=OFF -DBUILD_DOCS=OFF -DEnableCov=ON ${osv_def} ${lsb_def} ${os_def} ${dpv_def}

cmake --build build -j$(nproc)

cd $BUILD_DIR

./ut-DtkCore --gtest_output=xml:${XML_DIR}/report_dtkcore.xml

# find *.gcda from build dir
lcov -d ../ -c -o coverage_all.info
lcov --remove coverage_all.info "*/tests/*" "*/usr/include*" "*build/src*" "*build-ut/src*" --output-file coverage.info
cd ..
genhtml -o $HTML_DIR $BUILD_DIR/coverage.info && mv ${BUILD_DIR}/html/index.html ${BUILD_DIR}/html/cov_dtkcore.html

test -e ${BUILD_DIR}/asan.log* && mv ${BUILD_DIR}/asan.log* ${BUILD_DIR}/asan_dtkcore.log || touch ${BUILD_DIR}/asan.log

