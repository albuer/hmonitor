#!/bin/sh

NDK_DIR=~/Android/ndk/android-ndk-r14b

$NDK_DIR/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application.mk

echo "Output:"
mkdir -p out/android/
cp -av libs/* out/android/