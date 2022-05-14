export PATH=/opt/software/gcc-linaro-5.4.1-2017.05-x86_64_arm-linux-gnueabihf/bin:$PATH

\cp -r -f ../Paddle-Lite/build.lite.linux.armv7hf.gcc/inference_lite_lib.armlinux.armv7hf.intel_fpga/cxx/* ./Paddlelite/
\cp -f ../Paddle-Lite/intel_fpga_sdk/lib/build/libifcnna.so ./Paddlelite/lib
\cp -f ../nnadrv/build/nnadrv.ko .

cd ssd_detection_src
./build.sh