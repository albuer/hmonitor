#ifndef __GPU_NODE_H__
#define __GPU_NODE_H__

#include "FileUtils.h"

class GpuNode {
    int freq_; // 单位是 MHz
    int load_; // 百分比
    int temp_; // 单位是 1/1000 摄氏度
    char freq_path_[MAX_PATH];
    char load_path_[MAX_PATH];
    char temp_path_[MAX_PATH];
    char governor_path_[MAX_PATH];

public:
    GpuNode();
    int Update();

    int GetFreq() { return freq_; } // MHz
    float GetLoad() { return load_; }
    float GetTemp() { return (temp_>=0)?(temp_/1000.0f):temp_; }

    int Boost();
};

#endif