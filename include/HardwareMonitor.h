#ifndef __HARDWARE_MONITOR_H__
#define __HARDWARE_MONITOR_H__

#include "FileUtils.h"
#include "CpuNode.h"
#include "GpuNode.h"
#include "DdrNode.h"

class HardwareMonitor
{
protected:
    CpuNode cpu_;
    GpuNode gpu_;
    DdrNode ddr_;

    char android_ver_str_[256];
    char linux_ver_str_[512];
    char soc_[64];
    float fps_;
    FILE* logcat_fp_;

    int UpdateFPS();

public:
    HardwareMonitor();
    ~HardwareMonitor();
    int Update();

    int GetCpuFreq(int group=0) { return cpu_.GetFreq(group); } // MHz
    float GetCpuLoad(int group=0) { return cpu_.GetLoad(group); }
    int GetCpuIndex(int group=0) { return  cpu_.GetIndex(group); }
    float GetCpuTemp() { return cpu_.GetTemp(); }
    int GetCpuFreqAvg() { return cpu_.GetFreqAvg(); }
    float GetCpuLoadAvg() { return cpu_.GetLoadAvg(); }
    int GetCpuGroups() { return cpu_.GetGroups(); }

    int GetGpuFreq() { return gpu_.GetFreq(); } // MHz
    int GetGpuLoad() { return gpu_.GetLoad(); } 
    float GetGpuTemp() { return gpu_.GetTemp(); }

    int GetDdrFreq() { return ddr_.GetFreq(); } // MHz
    int GetDdrLoad() { return ddr_.GetLoad(); }

    float GetFPS() { return fps_; }

    const char* GetAndroidVersion() { return android_ver_str_; }
    const char* GetLinuxVersion() { return linux_ver_str_; }
    const char* GetChipModel() { return soc_; }
    int GetUpTime();

    int Boost();
    void DumpFreq();
};

#endif // __KERNEL_BASE_H__