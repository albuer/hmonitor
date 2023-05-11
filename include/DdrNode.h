#ifndef __DDR_NODE_H__
#define __DDR_NODE_H__

#include "FileUtils.h"

class DdrNode {
    int freq_; // 单位是 MHz
    int load_; // 百分比
    char freq_path_[MAX_PATH];
    char load_path_[MAX_PATH];
    char governor_path_[MAX_PATH];

public:
    DdrNode();
    int Update();

    int GetFreq() { return freq_; } // MHz
    float GetLoad() { return load_; }

    int Boost();
};

#endif