cp -r /home/bananasuper/work/Paddle-Lite/build.lite.linux.armv7hf.gcc/inference_lite_lib.armlinux.armv7hf.intel_fpga/cxx/include /home/bananasuper/work/ssd_detection_demo/Paddlelite
cp -r /home/bananasuper/work/Paddle-Lite/build.lite.linux.armv7hf.gcc/inference_lite_lib.armlinux.armv7hf.intel_fpga/cxx/lib /home/bananasuper/work/ssd_detection_demo/Paddlelite
cp /home/bananasuper/work/Paddle-Lite/intel_fpga_sdk/lib/build/libifcnna.so /home/bananasuper/work/ssd_detection_demo/Paddlelite/lib
rsync -rvlt --exclude-from=exclude.list  . root@$AIEP_HOST:/home/bananasuper/work/ssd_detection_demo
rsync -vt ../nnadrv/build/nnadrv.ko root@$AIEP_HOST:/home/bananasuper/work/ssd_detection_demo
