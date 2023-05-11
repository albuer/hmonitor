#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef ANDROID
#include <sys/system_properties.h>
#include <android/log.h>
#endif

#include "HardwareMonitor.h"

HardwareMonitor::HardwareMonitor() : fps_(-1), logcat_fp_(NULL)
{
    soc_[0] = linux_ver_str_[0] = android_ver_str_[0] = '\0';

    // get linux version
    memset(linux_ver_str_, 0, sizeof(linux_ver_str_));
    ReadFileAll("/proc/version", linux_ver_str_, sizeof(linux_ver_str_)-1);

#ifdef ANDROID
    // get android version
    __system_property_get("ro.build.version.release", android_ver_str_);

    // get chip model
    __system_property_get("ro.board.platform", soc_);

    // enable frame rate
    __system_property_set("debug.sf.fps", "1");


    // create logcat pipe
    logcat_fp_ = popen("logcat -b main -v time -s SurfaceFlinger", "r");
    if(logcat_fp_ != NULL) {
        // non block mode
        int fd = fileno(logcat_fp_);
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(fd, F_SETFL, flags);
    }
#endif
}

HardwareMonitor::~HardwareMonitor()
{
#ifdef ANDROID
    // disable frame rate
    __system_property_set("debug.sf.fps", "0");

    // close logcat pipe
    if (logcat_fp_ != NULL) {
        // 发送消息以唤醒logcat进程，从而才能顺序执行pclose
        __android_log_print(ANDROID_LOG_DEBUG, "SurfaceFlinger", "hmonitor");
        pclose(logcat_fp_);
    }
#endif
}

int HardwareMonitor::Update()
{
    cpu_.Update();
    gpu_.Update();
    ddr_.Update();

    UpdateFPS();

    return 0;
}

int HardwareMonitor::UpdateFPS()
{
    float fps = -1;
#ifdef ANDROID
    char buff[1024];
    memset(buff,0,sizeof(buff));

    if (logcat_fp_ != NULL) {
        fps = 0;
        while(fgets(buff, sizeof(buff), logcat_fp_))
        {
            char* p = strstr(buff, "mFps");
            if(p) {
                fps = atof(p+6);
            }
        }
    }
#endif

    fps_ = fps;

    return 0;
}

int HardwareMonitor::GetUpTime() {
    uint64_t uptime = 0;
    ReadFileValue("/proc/uptime", &uptime);

    return (int)uptime;
}

int HardwareMonitor::Boost()
{
    cpu_.Boost();
    gpu_.Boost();
    ddr_.Boost();

    return 0;
}

void HardwareMonitor::DumpFreq()
{
    printf("\nDump Freq:\n");
    for (int i=0; i<cpu_.GetGroups(); i++) {
        printf("    CPU[%d-%d]: %d MHz\n", cpu_.GetIndex(i), cpu_.GetIndex(i)+cpu_.GetCores(i)-1, cpu_.GetFreq(i));
    }

    printf("    GPU: %d MHz\n", gpu_.GetFreq());
    printf("    DDR: %d MHz\n", ddr_.GetFreq());
}