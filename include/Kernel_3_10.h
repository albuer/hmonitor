#ifndef __KERNEL_3_10_H__
#define __KERNEL_3_10_H__

void KernelBase::InitNode_3_10()
{
    // CPU TEMP
    cpu_temp_list.push_back(Node("/sys/devices/ff280000.tsadc/temp1_input", NULL, 1/1000.0f)); // rk3288: 1 ==> 1000
    cpu_temp_list.push_back(Node("/sys/rockchip_thermal/temp", NULL, 1/1000.0f)); // rk3368: 1 ==> 1000

    // GPU FREQ
    gpu_freq_list.push_back(Node("/sys/kernel/debug/clk/clk_gpu/clk_rate", NULL, 1000)); // rk3128/rk3288/rk3368, 未验证

    // GPU LOAD
    gpu_load_list.push_back(Node("/sys/devices/10091000.gpu/utilisation", NULL, 2.56)); // rk312x: 256 ==> 100
    gpu_load_list.push_back(Node("/sys/devices/ffa30000.gpu/dvfs", "utilisation")); //rk3288

    // GPU TEMP
    cpu_temp_list.push_back(Node("/sys/devices/ff280000.tsadc/temp2_input", NULL, 1/1000.0f)); // rk3288: 1 ==> 1000
    cpu_temp_list.push_back(Node("/sys/rockchip_thermal/temp", NULL, 1/1000.0f)); // rk3368: 与CPU一致

    // DDR FREQ
    ddr_freq_list.push_back(Node("/sys/kernel/debug/clk/clk_ddr/clk_rate", NULL, 1000)); // rk312x/rk3288/rk3368

    // DDR LOAD

    // VPU FREQ (KHz)
    vpu_freq_list.push_back(Node("/sys/kernel/debug/clk/clk_vdpu/clk_rate", NULL, 1000)); // rk312x
    vpu_freq_list.push_back(Node("/sys/kernel/debug/clk/clk_hevc_core/clk_rate", NULL, 1000)); // rk312x

    // VPU Enable, 注意列表顺序须与freq一致
    vpu_enable_list.push_back(Node("/sys/kernel/debug/clk/clk_vdpu/clk_enable_count")); // rk312x
    vpu_enable_list.push_back(Node("/sys/kernel/debug/clk/clk_hevc_core/clk_enable_count")); // rk312x
}

#endif //__KERNEL_3_10_H__