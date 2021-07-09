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
#endif

#include "KernelBase.h"
#include "Kernel_3_10.h"
#include "Kernel_4_4.h"
#include "Kernel_4_19.h"

/*
   0   成功
   <0  失败
 */
int KernelBase::Init()
{
    int ret = 0;

    memset(linux_ver_str_, 0, sizeof(linux_ver_str_));
    if (ReadFileAll("/proc/version", linux_ver_str_, sizeof(linux_ver_str_)-1) < 0) {
        return -1;
    }

    if (strncmp(linux_ver_str_+14, "3.10", 4) == 0) {
        InitNode_3_10();
    } else if (strncmp(linux_ver_str_+14, "4.4", 3) == 0) {
        InitNode_4_4();
    } else if (strncmp(linux_ver_str_+14, "4.19", 4) == 0) {
        InitNode_4_19();
    } else {
        InitNode_4_19();
    }

    ret = InitSoc();
    InitOS();

    gpu_freq_ = gpu_load_ = gpu_temp_ = 0;
    ddr_load_ = ddr_freq_ = vpu_freq_ = fps_ = 0;

    return ret;
}

/*
   /sys/devices/system/cpu/cpufreq/policy0
   /sys/devices/system/cpu/cpufreq/policy4
   ...
   /sys/devices/system/cpu/present                                                                                                                     
   0-5
 */
int KernelBase::InitSoc()
{
    int cpu_count = 0;
    char content[128];
    int begin, end;
    if (ReadFileAll("/sys/devices/system/cpu/present", content, sizeof(content)-1) < 0) {
        return -1;
    }
    sscanf(content, "%d-%d", &begin, &end);
    cpu_count = end-begin+1;

    // 统计cpu group 数量, 每个group的核数
    group_count_ = 0;
    char filename[512];
    for (int i=0; i<cpu_count; i++) {
        sprintf(filename, "/sys/devices/system/cpu/cpufreq/policy%d", i);
        if (access(filename, F_OK) == 0) {
            if (i > 0) {
                socinfo_[group_count_].cores = i - socinfo_[group_count_].index;
                ++group_count_;
            }
            socinfo_[group_count_].index = i;
            socinfo_[group_count_].total_cycle = socinfo_[group_count_].wake_cycle = 0;
            socinfo_[group_count_].last_total_cycle = socinfo_[group_count_].last_wake_cycle = 0;
        }
    }
    socinfo_[group_count_].cores = cpu_count - socinfo_[group_count_].index;
    ++group_count_;

    return 0;
}

int KernelBase::InitOS()
{
#ifdef ANDROID
// Android版本
    char value[256] = "";
    __system_property_get("ro.build.version.sdk", value);
    android_api_level_ = atoi(value);
    __system_property_get("ro.build.version.release", android_ver_str_);

    // for frame rate
    if (android_api_level_ > 20)
        __system_property_set("debug.sf.fps", "1");
    else
        __system_property_set("debug.hwc.logfps", "1");
#endif

    return 0;
}

int KernelBase::Update()
{
    UpdateCpu();
    UpdateGpu();
    UpdateDdr();
    UpdateVpu();
    UpdateFPS();

    return 0;
}

int KernelBase::UpdateCpu()
{
    if (group_count_ <= 0)
        return -1;

    UpdateCpuLoad();
    UpdateCpuFreq();
    UpdateCpuTemp();

    return 0;
}

int KernelBase::UpdateCpuLoad()
{
    FILE* fp = NULL;
    char line[256];
    int idx;
    long long user, nice, system, idle, iowait, irq, softirq;

    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
        return -1;

    for (int i=0; i<group_count_; i++) {
        socinfo_[i].last_wake_cycle = socinfo_[i].wake_cycle;
        socinfo_[i].last_total_cycle = socinfo_[i].total_cycle;
        socinfo_[i].wake_cycle = socinfo_[i].total_cycle = 0;
    }

    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "cpu%d %lld %lld %lld %lld %lld %lld %lld", &idx, &user,
                        &nice, &system, &idle, &iowait, &irq, &softirq)<8 ||
           line[3] == ' ')
            continue;

        struct soc_info* socinfo = NULL;
        for (int i=0; i<group_count_; i++) {
            if (idx >= socinfo_[i].index && idx < (socinfo_[i].index+socinfo_[i].cores)) {
                socinfo = &socinfo_[i];
                break;
            }
        }
        if (socinfo == NULL) {
            printf("cpu%d does not belong to any cpu group!\n", idx);
            continue;
        }

        long long used = user + nice + system + iowait + irq + softirq;
        socinfo->wake_cycle += used;
        socinfo->total_cycle += used + idle;
    }

    fclose(fp);

    for (int i=0; i<group_count_; i++) {
        // 第一次统计CPU负载
        if (socinfo_[i].last_wake_cycle == 0) {
            socinfo_[i].last_wake_cycle = socinfo_[i].wake_cycle;
            socinfo_[i].last_total_cycle = socinfo_[i].total_cycle;
        }
    }

    return 0;
}

int KernelBase::UpdateCpuFreq()
{
    for (int i=0; i<group_count_; i++) {
        char filename[512];
        sprintf(filename, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_cur_freq", socinfo_[i].index);
        ReadFileValue(filename, &socinfo_[i].freq);
    }
    
    return 0;
}

int KernelBase::UpdateCpuTemp()
{
    int temp = 0;
    ReadFListValue(cpu_temp_list, &temp);

    for (int i=0; i<group_count_; i++) {
        socinfo_[i].temp = temp;
    }
    
    return 0;
}

int KernelBase::UpdateGpu()
{
    ReadFListValue(gpu_freq_list, &gpu_freq_);
    ReadFListValue(gpu_load_list, &gpu_load_);
    ReadFListValue(gpu_temp_list, &gpu_temp_);

    return 0;
}

int KernelBase::UpdateDdr()
{
    ReadFListValue(ddr_freq_list, &ddr_freq_);
    ReadFListValue(ddr_load_list, &ddr_load_);

    return 0;
}

int KernelBase::UpdateVpu()
{
    int enable_count = 0;
    int ret = -1;

    vpu_freq_ = 0;
    for (size_t i=0; i<vpu_enable_list.size(); i++) {
        ret = ReadFileValue(vpu_enable_list[i].node, &enable_count);
        if (ret >= 0 && enable_count > 0) {
            ReadFileValue(vpu_freq_list[i], &vpu_freq_);
            break;
        }
    }

    return 0;
}

float KernelBase::GetCpuLoad(int group)
{
    float wake_delta = socinfo_[group].wake_cycle - socinfo_[group].last_wake_cycle;
    float total_delta = socinfo_[group].total_cycle - socinfo_[group].last_total_cycle;

    return (total_delta>0) ? (wake_delta*100.0f/total_delta) : 0;
}

int KernelBase::GetCpuFreqAvg()
{
    int freq = 0;
    int cores = 0;
    for (int i=0; i<group_count_; i++) {
        freq += socinfo_[i].freq * socinfo_[i].cores;
        cores += socinfo_[i].cores;
    }

    return (cores>0) ? (freq/1000/cores) : 0;
}

float KernelBase::GetCpuLoadAvg()
{
    float wake_delta = 0;
    float total_delta = 0;
    for (int i=0; i<group_count_; i++) {
        wake_delta += socinfo_[i].wake_cycle - socinfo_[i].last_wake_cycle;
        total_delta += socinfo_[i].total_cycle - socinfo_[i].last_total_cycle;
    }

    return (total_delta>0) ? (wake_delta*100.0f/total_delta) : 0;
}

int KernelBase::UpdateFPS()
{
    float fps = 0;
#ifdef ANDROID
    static char last_log_date[64] = "1";
    char cmd[128];

    if (android_api_level_ > 20)
        sprintf(cmd, "logcat -b main -v time -s SurfaceFlinger -t '%s'", last_log_date);
    else
        sprintf(cmd, "logcat -b main -v time -d -s hwcomposer -d");

    FILE *fp=NULL;
    char buff[1024];
    memset(buff,0,sizeof(buff));
    if(NULL==(fp=popen(cmd, "r")))
    {
        fprintf(stderr,"execute command failed: %s",strerror(errno));
        return -1;
    }

    while(fgets(buff, sizeof(buff), fp))
    {
        char* p = strstr(buff, "mFps");
        if(p) {
            fps = atof(p+6);
        }
    }
    pclose(fp);

    if (android_api_level_ > 20) {
        time_t timep;
        struct tm *p;
        time(&timep);
        p=localtime(&timep);
        sprintf(last_log_date, "%02d-%02d %02d:%02d:%02d.000", (1+p->tm_mon),
                    p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    //    printf("update last_log_date: %s\n", last_log_date);
    } else {
        system("logcat -c");
    }
#endif

    fps_ = fps;

    return 0;
}

int KernelBase::GetUpTime() {
    int uptime = 0;
    ReadFileValue("/proc/uptime", &uptime);

    return uptime;
}