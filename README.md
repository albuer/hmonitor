# hmonitor
Hardware Monitor for Rockchip platform

* **push the binary to devices**
```
adb push hmonitor /data/local/tmp/
adb shell chmod 0755 /data/local/tmp/hmonitor
```

* **usages**
```
1|root@rk3288:/ # /data/local/tmp/hmonitor

Hardware Monitor for Rockchip platform, Version: 2.0
	F - Freq(MHz)
	L - Load(%)
	T - Temperature(C)

Chip Model: rk3399
Android version: 8.1.0
Kernel: Linux version 4.4.126 (cmy@bogon) (gcc version 6.3.1 20170404 (Linaro GCC 6.3-2017.05) ) #1 SMP PREEMPT Wed Nov 21 12:35:21 CST 2018


UP(s)	CPU(F/L/T)	GPU(F/L/T)	VPU/HEVC(F)	DDR(F/L)    FPS
------------------------------------------------------------------------
001587	544/00/37	200/00/35	000/000		300/00	    0
  L(4)	408/00
  B(2)	816/01
001589	408/00/36	200/00/35	000/000		300/00	    0
  L(4)	408/00
  B(2)	408/01
001592	408/00/37	200/00/35	000/000		300/00	    0
  L(4)	408/00
  B(2)	408/00
001594	408/00/36	200/00/35	000/000		300/00	    0
  L(4)	408/00
  B(2)	408/01
```
