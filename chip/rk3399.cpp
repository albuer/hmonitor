#include "rk3399.h"

/*
    test platform: rk3399, android7.1, linux4.4
 */

RK3399Chip::RK3399Chip():ChipBase()
{
    lit_cpu_idx = 0;
    lit_cpu_count = 4;
    big_cpu_idx = 4;
    big_cpu_count = 2;
}

int RK3399Chip::get_cpu_freq_l()
{
    return get_freq_from_summary("armclkl");
}

int RK3399Chip::get_cpu_freq_b()
{
    return get_freq_from_summary("armclkb");
}

int RK3399Chip::get_cpu_temp()
{
    // 32222
    int temp = read_file_value("/sys/devices/virtual/thermal/thermal_zone0/temp");
    return temp/1000;
}

int RK3399Chip::get_gpu_freq()
{
    return get_freq_from_summary("aclk_gpu");
}

int RK3399Chip::get_gpu_load()
{
    // 21@200000000Hz
    int load = read_file_value("/sys/devices/platform/ff9a0000.gpu/devfreq/ff9a0000.gpu/load");
    return load;
}

int RK3399Chip::get_gpu_temp()
{
    // 32222
    int temp = read_file_value("/sys/devices/virtual/thermal/thermal_zone1/temp");
    return temp/1000;
}

int RK3399Chip::get_ddr_freq()
{
    return get_freq_from_summary("sclk_ddrc");
}

int RK3399Chip::get_vpu_freq()
{
    int freq, enable_cnt;
    freq = get_freq_from_summary("aclk_vcodec", &enable_cnt);
    if (enable_cnt <= 0) {
        freq = get_freq_from_summary("aclk_vdu", &enable_cnt);
        if (enable_cnt <= 0)
            freq = 0;
    }

    return freq;
}