LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= hmonitor

LOCAL_SRC_FILES := \
    src/main.cpp \
    src/FileUtils.cpp \
    src/HardwareMonitor.cpp \
    src/CpuNode.cpp \
    src/GpuNode.cpp \
    src/DdrNode.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

LOCAL_MODULE_TAGS := tests

LOCAL_CFLAGS += -Wall -Werror -Wno-error=deprecated-declarations -Wunused -Wunreachable-code
LOCAL_CPPFLAGS += -std=c++11 -O3
LOCAL_LDLIBS += -llog

LOCAL_32_BIT_ONLY := true

include $(BUILD_EXECUTABLE)
