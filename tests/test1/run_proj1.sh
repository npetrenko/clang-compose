#!/bin/bash
TESTBUILD=/tmp/proj1_testbuild

rm -rf $TESTBUILD

cmake -S proj1 -B $TESTBUILD
cp $TESTBUILD/compile_commands.json proj1/compile_commands.json
../../build/compose proj1/src/main.cpp --extra-arg=-I/usr/lib64/clang/9.0.0/include/ > /tmp/proj1_merged.cpp
diff /tmp/proj1_merged.cpp proj1_merged.cpp
# ../build/compose proj1/src/main.cpp > proj1_merged.cpp
