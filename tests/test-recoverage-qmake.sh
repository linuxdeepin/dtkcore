#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report
#EXTRACT_ARGS="src"
cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
qmake ..
make
cd ../tests/

rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR
qmake ../
make check

lcov -d ./ -c -o coverage_all.info
#lcov --extract coverage_all.info $EXTRACT_ARGS --output-file coverage.info
lcov --remove coverage_all.info "*/tests/*" "*/usr/include*" "*build/src*" --output-file coverage.info
cd ..
genhtml -o $REPORT_DIR $BUILD_DIR/coverage.info

#rm -rf $BUILD_DIR
#rm -rf ../$BUILD_DIR
