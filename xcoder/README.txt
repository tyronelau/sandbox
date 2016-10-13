1) cd 到 demo 目录下，make debug
2) 编译完毕后，cd 到 debug/bin/
3) export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../bin
4) export PATH=$PATH:../../../bin
5) ../demo --key YOUR_APP_ID --name YOUR_CHANNEL_NAME [--uid YOUR_UID]


头文件在 include/xcodec_interface.h 中
