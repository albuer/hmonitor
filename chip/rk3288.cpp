#include <stdio.h>
#include "rk3288.h"

/*
    test platform: rk3288, android4.4, linux3.10, linux4.4
 */

RK3288Chip::RK3288Chip():ChipBase()
{
    lit_cpu_idx = 0;
    lit_cpu_count = 4;
    big_cpu_idx = 0;
    big_cpu_count = 0;
}

int RK3288Chip::get_cpu_rate_l()
{
    return get_rate_from_summary(linux_ver==LINUX_3_10?"clk_core":"armclk");
}

int RK3288Chip::get_cpu_rate_b()
{
	return 0;
}

int RK3288Chip::get_cpu_temp()
{
    int temp = 0;

    if (linux_ver==LINUX_3_10) // 46
        temp = read_file_value("/sys/devices/ff280000.tsadc/temp1_input");
    else // 32222
        temp = read_file_value("/sys/devices/virtual/thermal/thermal_zone0/temp")/1000;

	return temp;
}

int RK3288Chip::get_gpu_rate()
{
	return get_rate_from_summary(linux_ver==LINUX_3_10?"clk_gpu":"sclk_gpu");
}

int RK3288Chip::get_gpu_load()
{
    int load = 0;

    if (linux_ver==LINUX_3_10) {
        /*
            mali DVFS is on
            utilisation:71
        */
        char line[256];
        FILE* fp = fopen("/sys/devices/ffa30000.gpu/dvfs", "r");
        if (fp) {
            while (fgets(line, sizeof(line), fp)) {
                if(strncmp(line, "utilisation", 11) == 0)
                    load = atoi(line+12);
            }
            fclose(fp);
        }
    } else { // 73@100000000Hz
        load = read_file_value("/sys/devices/platform/ffa30000.gpu/devfreq/ffa30000.gpu/load");
    }

	return load;
}

int RK3288Chip::get_gpu_temp()
{
    int temp = 0;

    if (linux_ver==LINUX_3_10) // 46
        temp = read_file_value("/sys/devices/ff280000.tsadc/temp2_input");
    else // 32222
        temp = read_file_value("/sys/devices/virtual/thermal/thermal_zone1/temp")/1000;

	return temp;
}

int RK3288Chip::get_ddr_rate()
{
	return get_rate_from_summary(linux_ver==LINUX_3_10?"clk_ddr":"sclk_ddrc");
}

int RK3288Chip::get_vcodec_rate()
{
    int rate, enable_cnt;
    rate = get_rate_from_summary(linux_ver==LINUX_3_10?"clk_vdpu":"aclk_vdpu", &enable_cnt);
    if (enable_cnt <= 0)
        rate = 0;

	return rate;
}

int RK3288Chip::get_hevc_rate()
{
    int rate, enable_cnt;
    rate = get_rate_from_summary(linux_ver==LINUX_3_10?"clk_hevc_core":"sclk_hevc_core", &enable_cnt);
    if (enable_cnt <= 0)
        rate = 0;

	return rate;
}


