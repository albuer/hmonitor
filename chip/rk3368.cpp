#include "rk3368.h"

/*
    test platform: rk3368, android8.1, linux4.4, linux3.10
 */

RK3368Chip::RK3368Chip():ChipBase()
{
    lit_cpu_idx = 0;
    lit_cpu_count = 4;
    big_cpu_idx = 4;
    big_cpu_count = 4;
}

int RK3368Chip::get_cpu_rate_l()
{
    return get_rate_from_summary(linux_ver==LINUX_3_10?"clk_core_l":"armclkl");
}

int RK3368Chip::get_cpu_rate_b()
{
    return get_rate_from_summary(linux_ver==LINUX_3_10?"clk_core_b":"armclkb");
}

int RK3368Chip::get_cpu_temp()
{
    int temp = 0;

    if (linux_ver==LINUX_3_10) // current cpu_temp:77
        temp = read_file_value("/sys/rockchip_thermal/temp");
    else // 32222
        temp = read_file_value("/sys/devices/virtual/thermal/thermal_zone0/temp")/1000;

    return temp;
}

int RK3368Chip::get_gpu_rate()
{
    return get_rate_from_summary(linux_ver==LINUX_3_10?"clk_gpu":"sclk_gpu_core");
}

int RK3368Chip::get_gpu_load()
{
    int load = 0;

    if (linux_ver==LINUX_3_10)
        load = 0;
    else // 21@200000000Hz
        load = read_file_value("/sys/devices/platform/ffa30000.rogue-g6110/devfreq/ffa30000.rogue-g6110/load");
    return load;
}

int RK3368Chip::get_gpu_temp()
{
    int temp = 0;

    if (linux_ver==LINUX_3_10)
        return get_cpu_temp();
    else // 32222
        temp = read_file_value("/sys/devices/virtual/thermal/thermal_zone1/temp")/1000;
    return temp;
}

int RK3368Chip::get_ddr_rate()
{
    return get_rate_from_summary(linux_ver==LINUX_3_10?"clk_ddr":"sclk_ddrc");
}

int RK3368Chip::get_vpu_rate()
{
    int rate, enable_cnt;
    rate = get_rate_from_summary("aclk_vdpu", &enable_cnt);
    if (enable_cnt <= 0) {
        rate = get_rate_from_summary(linux_ver==LINUX_3_10?"clk_hevc_core":"sclk_hevc_core", &enable_cnt);
        if (enable_cnt <= 0)
            rate = 0;
    }

    return rate;
}