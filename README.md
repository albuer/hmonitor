# hmonitor
Hardware Monitor for Rockchip platform

* **Build**

  change NDK_DIR in build_android.sh，and run it:

  ```
  $ ./build_android.sh
  [armeabi-v7a] Install        : hmonitor => libs/armeabi-v7a/hmonitor
  [arm64-v8a] Install        : hmonitor => libs/arm64-v8a/hmonitor
  Output:
  'libs/arm64-v8a/hmonitor' -> 'out/android/arm64-v8a/hmonitor'
  'libs/armeabi-v7a/hmonitor' -> 'out/android/armeabi-v7a/hmonitor'
  ```

* **Push the binary to devices**

  ```
  adb push hmonitor /data/local/tmp/
  adb shell chmod 0755 /data/local/tmp/hmonitor
  ```

* **Usages**

  ```
  # /data/local/tmp/hmonitor
  
  Hardware Monitor for Rockchip platform, Version: 3.0
  	F - Freq(MHz)
  	L - Load(%)
  	T - Temperature(C)
  
  Android version: 8.1.0
  Kernel: Linux version 4.4.126 (cmy@bogon) (gcc version 6.3.1 20170404 (Linaro GCC 6.3-2017.05) ) #1 SMP PREEMPT Wed Nov 21 12:35:21 CST 2018
  
  
  UP(s)	CPU(F|L|T)	GPU(F|L|T)	VPU(F)	DDR(F|L)	FPS
  ------------------------------------------------------------------------
    2631	1200|00.0|35.0	 200|00|31.9	  0	 324|00		0.0
    2633	1200|01.2|34.4	 200|00|31.9	  0	 324|00		0.0
    2635	 408|01.0|35.0	 200|00|31.9	  0	 324|00		0.0
    2637	 408|01.3|34.4	 200|00|31.9	  0	 324|00		0.0
    2639	 408|01.0|35.0	 200|00|31.9	  0	 324|00		0.0
    2641	 408|01.5|35.0	 200|00|31.9	  0	 324|00		0.0
    2643	 408|01.3|35.0	 200|00|32.5	  0	 324|00		0.0
  ```
