#!/bin/bash

SRC=" src/main.cpp src/HardwareMonitor.cpp src/FileUtils.cpp src/CpuNode.cpp src/GpuNode.cpp src/DdrNode.cpp"

mkdir -p out/linux/armeabi-v7a &&
arm-linux-gnueabihf-g++ -static -o out/linux/armeabi-v7a/hmonitor -I./include $SRC &&
echo "output: out/linux/armeabi-v7a/hmonitor"

mkdir -p out/linux/arm64-v8a &&
aarch64-linux-gnu-g++ -static -o out/linux/arm64-v8a/hmonitor -I./include $SRC &&
echo "output: out/linux/arm64-v8a/hmonitor"
