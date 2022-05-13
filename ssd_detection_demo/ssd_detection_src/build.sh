#!/bin/bash

# configure
TARGET_ARCH_ABI=armv7hf # for Cyclone V SoC
PADDLE_LITE_DIR=../Paddlelite

if [ "x$1" != "x" ]; then
    TARGET_ARCH_ABI=$1
fi

# build
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DLITE_WITE_PROFILE=1 -DPADDLE_LITE_DIR=${PADDLE_LITE_DIR} -DTARGET_ARCH_ABI=${TARGET_ARCH_ABI} -DCMAKE_PREFIX_PATH=../Paddlelite/lib .. 
make

