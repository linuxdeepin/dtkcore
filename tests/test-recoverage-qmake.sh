#!/bin/bash

BUILD_DIR=`pwd`/../build-ut
HTML_DIR=${BUILD_DIR}/html
XML_DIR=${BUILD_DIR}/report
#EXTRACT_ARGS="src"
cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
qmake .. CONFIG+=debug
make -j$(nproc)
cd ../tests/

rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
qmake ../ CONFIG+=debug
export ASAN_OPTIONS=halt_on_error=0
TESTARGS="--gtest_output=xml:${XML_DIR}/report_dtkcore.xml"  make check -j$(nproc)

lcov -d ./ -c -o coverage_all.info
#lcov --extract coverage_all.info $EXTRACT_ARGS --output-file coverage.info
lcov --remove coverage_all.info "*/tests/*" "*/usr/include*" "*build/src*" "*build-ut/src*" --output-file coverage.info
cd ..
genhtml -o $HTML_DIR $BUILD_DIR/coverage.info && mv ${BUILD_DIR}/html/index.html ${BUILD_DIR}/html/cov_dtkcore.html

test -e ${BUILD_DIR}/asan.log* && mv ${BUILD_DIR}/asan.log* ${BUILD_DIR}/asan_dtkcore.log || touch ${BUILD_DIR}/asan.log

