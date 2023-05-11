#include "DdrNode.h"

DdrNode::DdrNode()
{
    freq_ = load_ = -1;
    freq_path_[0] = load_path_[0] = governor_path_[0] = '\0';

    /*
      若/sys/class/devfreq/dmc存在，则表明有开启了动态变频，此时可从该目录下面节点获取频率/负载信息
      否则，可从下列的clk节点获得频率信息，无法获得负载信息，也不能修改频率：
          1: "/sys/kernel/debug/clk/sclk_ddrc/clk_rate"
          2: "/sys/kernel/debug/clk/clk_scmi_ddr/clk_rate"
          2: "/sys/kernel/debug/clk/scmi_clk_ddr/clk_rate"
    */
    if (access("/sys/class/devfreq/dmc/cur_freq", F_OK) == 0) {
        sprintf(freq_path_, "/sys/class/devfreq/dmc/cur_freq");
        sprintf(load_path_, "/sys/class/devfreq/dmc/load");
        sprintf(governor_path_, "/sys/class/devfreq/dmc/governor");
    } else if (access("/sys/kernel/debug/clk/sclk_ddrc/clk_rate", F_OK) == 0) {
        sprintf(freq_path_, "/sys/class/devfreq/dmc/cur_freq");
    } else if (access("/sys/kernel/debug/clk/clk_scmi_ddr/clk_rate", F_OK) == 0) {
        sprintf(freq_path_, "/sys/kernel/debug/clk/clk_scmi_ddr/clk_rate");
    } else if (access("/sys/kernel/debug/clk/scmi_clk_ddr/clk_rate", F_OK) == 0) {
        sprintf(freq_path_, "/sys/kernel/debug/clk/scmi_clk_ddr/clk_rate");
    }
}

int DdrNode::Update()
{
    uint64_t value = 0;
    freq_ = load_ = -1;

    if (freq_path_[0] != 0 && ReadFileValue(freq_path_, &value) >= 0) {
        freq_ = (int)(value/1000000); // MHz
    }

    if (load_path_[0] != 0 && ReadFileValue(load_path_, &value) >= 0) {
        load_ = (int)value;
    }

    return 0;
}

int DdrNode::Boost()
{
    int ret = WriteFileAll(governor_path_, "performance");
    printf("Boost DDR: %s\n", (ret>0)?"Success":"Failure");

    return ret;
}