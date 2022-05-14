#!/bin/bash
TARGET_ARCH_ABI=armhf
if [ -n "$1" ]; then
    TARGET_ARCH_ABI=$1
fi

function readlinkf() {
    perl -MCwd -e 'print Cwd::abs_path shift' "$1";
}

rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET_ARCH_ABI=${TARGET_ARCH_ABI} ..
make
cp libifcnna.so ../
mv ../libifcnna.so ../libvnna.so
