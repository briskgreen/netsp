netsp
=====

一个简单的流量监控程序

Linux下的一个简单的流量监控程序，通过解析/proc/net/dev文件获得数据
显示内容包括当前某个网卡接口上传/下载的流量/数据包量，以及某段时间
上传/下载的速度评估

编译

gcc -o netsp netsp.c -lpthread -lncursesw
