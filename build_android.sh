#!/bin/sh

NDK_DIR=~/Android/Sdk/ndk/16.1.4479499

rm -rf obj/ libs/
$NDK_DIR/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application.mk

echo "Output:"
mkdir -p out/android/
cp -av libs/* out/android/
