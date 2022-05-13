export LD_LIBRARY_PATH=/opt/ssd_detection_demo/Paddlelite/lib:/opt/ssd_detection_demo/ocv3.4.10/lib
gdbserver :10000 ssd_detection_src/build/ssd_detection config_ssd_mobilenet_v1.txt images/road554.png
