#!/bin/sh

NDK_DIR=~/WORK/android-tools/android-ndk-r10d

$NDK_DIR/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application.mk
