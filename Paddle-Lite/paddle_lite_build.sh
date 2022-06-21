
cd intel_fpga_sdk/lib
./build.sh
cd ../..

rm -rf third_party
rm -rf build.opt
sudo rm -rf build.lite.linux.armv7hf.gcc


# ./lite/tools/build.sh build_optimize_tool

./lite/tools/build_linux.sh --arch=armv7hf --with_extra=ON --with_log=ON --with_intel_fpga=ON --intel_fpga_sdk_root=./intel_fpga_sdk full_publish