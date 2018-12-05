
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= hmonitor

LOCAL_SRC_FILES := \
    main.cpp \
    chip/ChipBase.cpp \
    chip/rk3399.cpp \
    chip/rk3288.cpp \
    chip/rk312x.cpp \
    chip/rk3368.cpp

LOCAL_SHARED_LIBRARIES :=  \
    libc \
    libcutils \
    libutils

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/chip

LOCAL_MODULE_TAGS := tests

LOCAL_CFLAGS += -Wall -Werror -Wno-error=deprecated-declarations -Wunused -Wunreachable-code
#LOCAL_CFLAGS += -Werror

LOCAL_32_BIT_ONLY := true

include $(BUILD_EXECUTABLE)
