#!/bin/bash

mkdir -p out/linux/armeabi-v7a &&
arm-linux-gnueabihf-g++ -static -o out/linux/armeabi-v7a/hmonitor -I./include src/KernelBase.cpp src/hmonitor.cpp src/FileUtils.cpp &&
echo "output: out/linux/armeabi-v7a/hmonitor"

mkdir -p out/linux/arm64-v8a &&
aarch64-linux-gnu-g++ -static -o out/linux/arm64-v8a/hmonitor -I./include src/KernelBase.cpp src/hmonitor.cpp src/FileUtils.cpp &&
echo "output: out/linux/arm64-v8a/hmonitor"
