1) cd 到 demo 目录下，make debug
2) 编译完毕后，cd 到 debug/bin/
3) export PATH=$PATH:../../../bin
4) ../demo --key YOUR_APP_ID --name YOUR_CHANNEL_NAME [--uid YOUR_UID]

头文件在 include/xcodec_interface.h 中
linux系统要求
内核 >= 2.6.32
glibc >= 2.12

编译器要求 gcc 4.4 及以上

1) xcoder 的搜索路径设置
 export PATH=$PATH:../../bin (PATH 路径里要包含xcoder所在的路径，因为Server SDK 是通过真正的视频转码程序xcoder 工作的）

2) 日志的配置和检索
 服务的日志都是通过系统API syslog 打印输出，对问题的追查需要通过系统日志。centos 的默认配置是输出到 /var/log/messages

3) 音视频的输出
  对音视频数据，可以分别设置解码和不解码模式
  a) 音频不解码的话，AudioFrameReceived 收到的音频帧为每个用户的AAC格式，解码的话，将是原始的PCM格式(每个用户的PCM数据和混以后的PCM数据）
  b) 视频不解码的话，VideoFrameReceived 收到的视频是每个用户的h264 原始数据帧；解码的话收到的是每个用户的YUV格式数据帧

4) 该 SDK 仅支持观众模式，不能发送音视频数据。每个实例加入到一个指定的频道，该频道内的所有主播（发送音视频者）的实时数据会通过上述的
AudioFrameReceived/VideoFrameReceived 推送给实例的创建者

5) UDP 端口的设置
  a) Agora SDK 是通过 UDP 实现数据的传输，需要在部署服务器上开放指定的 UDP 端口，需要开放 Agora 服务端侦听的UDP 端口:
     4001-4010， 25000， 8000.
  部署的服务器本地UDP 端口也需要打开，因为目前 Agora SDK 暂未提供指定本地 UDP 端口的功能，测试和部署期间，
  本地UDP端口建议都打开，以防止出现收不到数据的问题
