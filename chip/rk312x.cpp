#include "rk312x.h"

/*
    test platform: rk3126c, android7.1, linux3.10
 */

RK312xChip::RK312xChip():ChipBase()
{
    lit_cpu_idx = 0;
    lit_cpu_count = 4;
    big_cpu_idx = 0;
    big_cpu_count = 0;
}

int RK312xChip::get_cpu_freq_l()
{
    return get_freq_from_summary("clk_core");
}

int RK312xChip::get_cpu_freq_b()
{
    return 0;
}

int RK312xChip::get_cpu_temp()
{
    // unsupport
    return 0;
}

int RK312xChip::get_gpu_freq()
{
    return get_freq_from_summary("clk_gpu");
}

int RK312xChip::get_gpu_load()
{
    // 256 ==> 100%
    int load = read_file_value("/sys/devices/10091000.gpu/utilisation");

    return (load*100)/256;
}

int RK312xChip::get_gpu_temp()
{
    // unsupport
    return 0;
}

int RK312xChip::get_ddr_freq()
{
    return get_freq_from_summary("clk_ddr");
}

int RK312xChip::get_vpu_freq()
{
    int freq, enable_cnt;
    freq = get_freq_from_summary("clk_vdpu", &enable_cnt);
    if (enable_cnt <= 0) {
        freq = get_freq_from_summary("clk_hevc_core", &enable_cnt);
        if (enable_cnt <= 0)
            freq = 0;
    }

    return freq;
}