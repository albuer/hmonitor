#ifndef __CPU_NODE_H__
#define __CPU_NODE_H__

#include "FileUtils.h"

#define MAX_GROUP       10
struct CpuGroups {
    int index; // 这组cpu从哪个核开始，以0开始计数
    int cores; // 这组cpu有多少个核
    int freq; // 单位是 MHz
    long long wake_cycle;
    long long total_cycle;
    long long last_wake_cycle;
    long long last_total_cycle;
};

class CpuNode {
    struct CpuGroups groups_[MAX_GROUP];
    int group_count_;
    char temperature_path_[MAX_PATH];
    int temperature_;

public:
    CpuNode();

    int Update();
    int UpdateLoad();
    int UpdateFreq();
    int UpdateTemp();

    int GetFreq(int group=0) { return groups_[group].freq; } // MHz
    float GetLoad(int group=0);
    float GetTemp() { return temperature_/1000.0f; }

    int GetFreqAvg(); // MHz
    float GetLoadAvg();

    int GetIndex(int group=0) { return  groups_[group].index; }
    int GetCores(int group=0) { return  groups_[group].cores; }
    int GetGroups() { return  group_count_; }

    int Boost();
};

#endif