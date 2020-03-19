#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

#include "rk3399.h"
#include "rk3288.h"
#include "rk312x.h"
#include "rk3368.h"

#define APP_VERSION     "2.0"

ChipBase* chip = NULL;

int get_uptime(void)
{
    int uptime = 0;
    char line[256];
    FILE* fp = fopen("/proc/uptime", "r");
    if (fp) {
        if (fgets(line, sizeof(line), fp)) {
            uptime = atoi(line);
        }
        fclose(fp);
    }

    return uptime;
}

int main()
{
    printf("Hardware Monitor for Rockchip platform, Version: %s\n", APP_VERSION);
    printf("\tF - Freq(MHz)\n");
    printf("\tL - Load(%%)\n");
    printf("\tT - Temperature(C)\n");
    printf("\n");

    int chip_model = ChipBase::get_model();
    printf("Chip Model: %s\n", ChipBase::chip_model_str);

    switch(chip_model) {
    case CHIP_3399:
        chip = new RK3399Chip();
        break;
    case CHIP_3288:
        chip = new RK3288Chip();
        break;
    case CHIP_312x:
        chip = new RK312xChip();
        break;
    case CHIP_3368:
        chip = new RK3368Chip();
        break;
    default:
        printf("unsupport chip!");
        return -1;
    }

    printf("Android version: %s\n", chip->android_ver_str);
    printf("Kernel: %s\n", chip->linux_ver_str);

    printf("\nUP(s)\tCPU(F/L/T)\tGPU(F/L/T)\tVPU(F)\tDDR(F/L)    FPS\n");
    printf("------------------------------------------------------------------------\n");

    int loop_count = 0;

    while (1) {
        // print clk info
        int uptime = get_uptime();

        chip->update_clk_summary();
        int lit_cpu_load, lit_cpu_freq;
        int big_cpu_load, big_cpu_freq;
        chip->get_cpu_load(&lit_cpu_load, &big_cpu_load);
        lit_cpu_freq = chip->get_cpu_freq_l();
        big_cpu_freq = chip->get_cpu_freq_b();

        int cpu_freq, cpu_load;
        cpu_freq = (big_cpu_freq*chip->big_cpu_count + lit_cpu_freq*chip->lit_cpu_count)/(chip->big_cpu_count+chip->lit_cpu_count);
        cpu_load = (big_cpu_load*chip->big_cpu_count + lit_cpu_load*chip->lit_cpu_count)/(chip->big_cpu_count+chip->lit_cpu_count);
        printf("%.6d\t%03d/%02d/%02d\t%03d/%02d/%02d\t%03d\t%03d/%02d\t    %d\n", uptime,
                cpu_freq, cpu_load, chip->get_cpu_temp(),
                chip->get_gpu_freq(), chip->get_gpu_load(), chip->get_gpu_temp(),
                chip->get_vpu_freq(),
                chip->get_ddr_freq(), chip->get_ddr_load(), chip->get_fps());

        if (chip->big_cpu_count > 0) {
            printf("  L(%d)\t%03d/%02d\n", chip->lit_cpu_count, lit_cpu_freq, lit_cpu_load);
            printf("  B(%d)\t%03d/%02d\n", chip->big_cpu_count, big_cpu_freq, big_cpu_load);
        }

        // delay 2s
        usleep(2000000);
        if(++loop_count>=16) {
            loop_count = 0;
            printf("\nUP(s)\tCPU(F/L/T)\tGPU(F/L/T)\tVPU(F)\tDDR(F/L)    FPS\n");
            printf("------------------------------------------------------------------------\n");
        }
    }

//    printf("End.\n");

    return 0;
}

