#ifndef __KERNEL_4_19_H__
#define __KERNEL_4_19_H__

void KernelBase::InitNode_4_19()
{
    // CPU TEMP
    cpu_temp_list.push_back(Node("/sys/class/thermal/thermal_zone0/temp"));

    // GPU FREQ (KHz)
    gpu_freq_list.push_back(Node("/sys/class/devfreq/ffa30000.rogue-g6110/cur_freq", NULL, 1000)); // rk3368

    // GPU LOAD
    gpu_load_list.push_back(Node("/sys/class/devfreq/ffa30000.rogue-g6110/load")); // rk3368

    // GPU TEMP
    gpu_temp_list.push_back(Node("/sys/class/thermal/thermal_zone1/temp"));

    // DDR FREQ (KHz)
    ddr_freq_list.push_back(Node("/sys/class/devfreq/dmc/cur_freq", NULL, 1000));
    ddr_freq_list.push_back(Node("/sys/kernel/debug/clk/sclk_ddrc/clk_rate", NULL, 1000));

    // DDR LOAD
    ddr_load_list.push_back(Node("/sys/class/devfreq/dmc/load"));

    /*
       在目前的linux4.19版本中， 不确定是从哪个节点获取vpu频率
     */
    // VPU FREQ (KHz)
    //vpu_freq_list.push_back(Node("/sys/kernel/debug/clk/aclk_rkvdec/clk_rate", NULL, 1000)); // rk3566
    //vpu_freq_list.push_back(Node("/sys/kernel/debug/clk/aclk_vcodec_noc/clk_rate", NULL, 1000)); // rk3399

    // VPU Enable, 注意列表顺序须与freq一致
    //vpu_enable_list.push_back(Node("/sys/kernel/debug/clk/aclk_rkvdec/clk_enable_count")); // rk3566
    //vpu_enable_list.push_back(Node("/sys/kernel/debug/clk/aclk_vcodec_noc/clk_enable_count")); // rk3399
}

#endif //__KERNEL_4_19_H__