#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "KernelBase.h"

#define APP_VERSION     "3.0"

int main(int argc, const char** argv)
{
    printf("Hardware Monitor for Rockchip platform, Version: %s\n", APP_VERSION);
    printf("\tF - Freq(MHz)\n");
    printf("\tL - Load(%%)\n");
    printf("\tT - Temperature(C)\n");
    printf("\n");

    float delay = 2*1000000;
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && (i+1)<argc) {
            delay = atof(argv[i+1])*1000000;
            ++i;
        }
    }

    KernelBase base;
    base.Init();

#ifdef ANDROID
    printf("Android version: %s\n", base.GetAndroidVersion());
#endif
    printf("Kernel: %s\n", base.GetLinuxVersion());

    printf("\nUP(s)\tCPU(F|L|T)\tGPU(F|L|T)\tVPU(F)\tDDR(F|L)\tFPS\n");
    printf("------------------------------------------------------------------------\n");

    int loop_count = 0;

    while (1) {
        base.Update();

        // print clk info
        int uptime = base.GetUpTime();

        printf("%6d\t%4d|%04.1f|%04.1f\t%4d|%02d|%04.1f\t%3d\t%4d|%02d\t\t%.1f\n", uptime,
                base.GetCpuFreqAvg(), base.GetCpuLoadAvg(), base.GetCpuTemp(),
                base.GetGpuFreq(), base.GetGpuLoad(), base.GetGpuTemp(),
                base.GetVpuFreq(), base.GetDdrFreq(), base.GetDdrLoad(),
                base.GetFPS());

        int cpu_groups = base.GetCpuGroups();
        if (cpu_groups > 1) {
            for (int i=0; i<cpu_groups; i++) {
                printf("   (%d)\t%4d|%4.1f\n", base.GetCpuCores(i), base.GetCpuFreq(i), base.GetCpuLoad(i));
            }
        }

        usleep(delay);
        if(++loop_count>=10) {
            loop_count = 0;
            printf("\nUP(s)\tCPU(F|L|T)\tGPU(F|L|T)\tVPU(F)\tDDR(F|L)\tFPS\n");
            printf("------------------------------------------------------------------------\n");
        }
    }

    return 0;
}
