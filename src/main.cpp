#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "HardwareMonitor.h"

#define APP_VERSION     "4.0"

bool is_quit = false;
static void my_signal_handler(int sig)
{
    is_quit = true;
}

int main(int argc, const char** argv)
{
    printf("Hardware Monitor for Rockchip platform, Version: %s\n", APP_VERSION);
    printf("\n");

    float delay = 1*1000000;
    bool do_boost = false;
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && (i+1)<argc) {
            delay = atof(argv[i+1])*1000000;
            ++i;
        } else if (strcmp(argv[i], "-b") == 0) {
            do_boost = true;
        } else {
            printf("Usage: %s [-d SECONDS] [-b] [-h]\n\n", argv[0]);
            printf("-d   Delay SECONDS between each cycle (default 1)\n");
            printf("-b   Enter boost mode\n");
            printf("-h   Print this help\n");
            return 0;
        }
    }

    signal(SIGTERM, &my_signal_handler);
    signal(SIGINT, &my_signal_handler);
    signal(SIGQUIT, &my_signal_handler);
    signal(SIGKILL, &my_signal_handler);

    HardwareMonitor hwm;

    if (do_boost) {
        hwm.Boost();
        usleep(100*1000); // make system idle
        hwm.Update();
        hwm.DumpFreq();
        return 0;
    }

    printf("CHIP MODEL  : %s\n", hwm.GetChipModel());
#ifdef ANDROID
    printf("ANDROID VER : %s\n", hwm.GetAndroidVersion());
#endif
    printf("KERNEL VER  : %s\n", hwm.GetLinuxVersion());

    int countdown = 0;

    hwm.Update();
    while (is_quit==false) {
        if (--countdown <= 0) {
            countdown = 20;

            // print title
            printf("\n");
            printf("               CPU               GPU             DDR\n");
            printf(" UP(s)    Freq|Load|Temp    Freq|Load|Temp    Freq|Load      FPS\n");
            printf("-----------------------------------------------------------------\n");
        }

        usleep(delay);
        if (is_quit) break;

        hwm.Update();

        // print clk info
        int uptime = hwm.GetUpTime();

        printf("%6d    %4d|%4.1f|%4.1f    %4d|%4d|%4.1f    %4d|%4d    %5.1f\n", uptime,
                hwm.GetCpuFreqAvg(), hwm.GetCpuLoadAvg(), hwm.GetCpuTemp(),
                hwm.GetGpuFreq(), hwm.GetGpuLoad(), hwm.GetGpuTemp(),
                hwm.GetDdrFreq(), hwm.GetDdrLoad(), hwm.GetFPS());

        int cpu_groups = hwm.GetCpuGroups();
        if (cpu_groups > 1) {
            for (int i=0; i<cpu_groups; i++) {
                printf("  CPU%d    %4d|%4.1f\n", hwm.GetCpuIndex(i), hwm.GetCpuFreq(i), hwm.GetCpuLoad(i));
            }
        }
    }

    return 0;
}
