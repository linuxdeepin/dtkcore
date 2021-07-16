#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report
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
TESTARGS="--gtest_output=xml:dde_test_report_dtkcore.xml"  make check -j$(nproc)

lcov -d ./ -c -o coverage_all.info
#lcov --extract coverage_all.info $EXTRACT_ARGS --output-file coverage.info
lcov --remove coverage_all.info "*/tests/*" "*/usr/include*" "*build/src*" --output-file coverage.info
cd ..
genhtml -o $REPORT_DIR $BUILD_DIR/coverage.info

#rm -rf $BUILD_DIR
#rm -rf ../$BUILD_DIR

test -e ./build/asan.log* && mv ./build/asan.log* ./build/asan_dtkcore.log || touch ./build/asan.log

