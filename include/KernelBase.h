#ifndef __KERNEL_BASE_H__
#define __KERNEL_BASE_H__

#include "FileUtils.h"

#define MAX_GROUP       10
struct soc_info {
    int index; // 这组cpu从哪个核开始，以0开始计数
    int cores; // 这组cpu有多少个核
    int freq; // 单位是 KHz
    int temp; // 单位是 1/1000 摄氏度
    long long wake_cycle;
    long long total_cycle;
    long long last_wake_cycle;
    long long last_total_cycle;
};

class KernelBase
{
protected:
    std::vector<Node> cpu_temp_list;
    std::vector<Node> gpu_freq_list;
    std::vector<Node> gpu_load_list;
    std::vector<Node> gpu_temp_list;
    std::vector<Node> ddr_freq_list;
    std::vector<Node> ddr_load_list;
    std::vector<Node> vpu_enable_list;
    std::vector<Node> vpu_freq_list;

    struct soc_info socinfo_[MAX_GROUP];
    int group_count_;

    int gpu_freq_; // 单位是 KHz
    int gpu_load_; // 百分比
    int gpu_temp_; // 单位是 1/1000 摄氏度

    int ddr_freq_; // 单位是 KHz
    int ddr_load_; // 百分比

    int vpu_freq_; // 单位是 KHz

    float fps_;

    int android_api_level_;
    char android_ver_str_[256];
    char linux_ver_str_[512];

    int InitSoc();
    int InitOS();

    int UpdateCpu();
    int UpdateCpuLoad();
    int UpdateCpuFreq();
    virtual int UpdateCpuTemp();

    virtual int UpdateGpu();
    virtual int UpdateDdr();
    virtual int UpdateVpu();
    int UpdateFPS();

    void InitNode_4_19();
    void InitNode_3_10();
    void InitNode_4_4();

public:
    int Init();
    int Update();

    float GetCpuLoad(int group=0);
    int GetCpuCores(int group=0) { return socinfo_[group].cores; }
    int GetCpuFreq(int group=0) { return socinfo_[group].freq/1000; } // MHz
    float GetCpuTemp(int group=0) { return socinfo_[group].temp/1000.0f; }
    int GetCpuFreqAvg();
    float GetCpuLoadAvg();
    int GetCpuGroups() { return group_count_; }

    int GetGpuLoad() { return gpu_load_; }
    int GetGpuFreq() { return gpu_freq_/1000; } // MHz
    float GetGpuTemp() { return gpu_temp_/1000.0f; }

    int GetDdrLoad() { return ddr_load_; }
    int GetDdrFreq() { return ddr_freq_/1000; } // MHz

    int GetVpuFreq() { return vpu_freq_/1000; } // MHz

    float GetFPS() { return fps_; }

    const char* GetAndroidVersion() { return android_ver_str_; }
    const char* GetLinuxVersion() { return linux_ver_str_; }
    int GetUpTime();
};

#endif // __KERNEL_BASE_H__