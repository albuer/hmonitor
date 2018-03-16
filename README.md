# hmonitor
Hardware Monitor in Rockchip platform
主要监视CPU/GPU频率和温度等信息

* **把脚本文件推送到设备中**
```
adb push hmonitor.sh /data/local/tmp/
adb shell chmod 0755 /data/local/tmp/hmonitor.sh
```

* **用法**
```
/data/local/tmp/hmonitor.sh -h
...
Usage: /data/local/tmp/hmonitor.sh [OPTION]...
  -C clock		Setup CPU frequency at MHz
  -G clock		Setup GPU frequency at MHz
  -D clock		Setup DDR frequency at MHz
  -V 			for VR product
  -n num		Loop count
```

使用hmonitor监视5秒然后结束
```
1|root@rk3288:/ # /data/local/tmp/hmonitor.sh -n 5                             

Hardware Monitor for RK3288 VR, Version: 1.7
	F - Freq(MHz)
	L - Load(%)
	T - Temperature(C)

[Model]: rk3288
[Firmware]: rk3288-userdebug 5.1.1 LMY49F eng.zj.20161031.172714 test-keys
[Kernel]: Linux version 3.10.0 (zj@RD-DEP1-SERVER-163) (gcc version 4.6.x-google 20120106 (prerelease) (GCC) ) #1 SMP PREEMPT Mon Oct 24 16:48:19 CST 2016

Set Loop number: 5

UPTIME(s)	CPU(F/L/T)	GPU(F/L/T)	VPU/HEVC(F)	DDR(F/L)	FPS
00:12:49	312/07/33	099/00/32	000/000		396/00		0.0
00:12:50	312/11/34	099/00/33	000/000		396/00		0.0
00:12:51	312/11/34	099/00/33	000/000		396/00		0.0
00:12:52	600/10/34	099/00/33	000/000		396/00		0.0
00:12:53	600/11/34	099/00/33	000/000		396/00		0.0

UPTIME(s)	CPU(F/L/T)	GPU(F/L/T)	VPU/HEVC(F)	DDR(F/L)	FPS
Average(5):	427/10/34	099/00/33	000/000		396/00		0.0
Fixed freq: CPU=0 GPU=0 DDR=0
```
