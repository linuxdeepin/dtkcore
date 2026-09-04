#!/bin/bash

# SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
set -ex

# Thin wrapper around the cmake "coverage" target. The target already runs the
# tests, collects the profile data and generates the HTML report, so this script
# only configures the build and forwards the artifacts the CI job expects -
# keeping the lcov/genhtml invocations here in sync with the target would just
# duplicate them.
BUILD_DIR=$(pwd)/../build
REPORT_DIR=${BUILD_DIR}/tests

export ASAN_OPTIONS="halt_on_error=0"

# back to project directroy
cd ..

cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr \
      -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DBUILD_TESTING=ON \
      -DEnableCov=ON -DDSYSINFO_PREFIX=/tmp
cmake --build build --target coverage

mkdir -p ${REPORT_DIR}
rm -rf ${REPORT_DIR}/html
mv ${BUILD_DIR}/coverage_report ${REPORT_DIR}/html

test -e ${REPORT_DIR}/asan.log* && mv ${REPORT_DIR}/asan.log* ${REPORT_DIR}/asan_dtkcore.log || touch ${REPORT_DIR}/asan.log
