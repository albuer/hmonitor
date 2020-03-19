#include <sys/system_properties.h>
#include "ChipBase.h"

struct clk_info* ChipBase::clk_summary = new struct clk_info[1000];
int ChipBase::clk_summary_idx = 0;
char ChipBase::chip_model_str[256] = "";

ChipBase::ChipBase()
{
    char value[256] = "";
    __system_property_get("ro.build.version.sdk", value);
    android_api_level = atoi(value);
    __system_property_get("ro.build.version.release", android_ver_str);
//    printf("ro.build.version.release: %s\n", value);

    // get kernel version
    FILE* fp = fopen("/proc/version", "r");
    if (fp) {
        if (fgets(linux_ver_str, sizeof(linux_ver_str), fp)) {
//            printf("kernel version: %s\n", value);
            if (strncmp(linux_ver_str+14, "4.4", 3) == 0) {
                linux_ver = LINUX_4_4;
            } else if (strncmp(linux_ver_str+14, "3.10", 4) == 0) {
                linux_ver = LINUX_3_10;
            } else {
                linux_ver = LINUX_UNKNOWN;
            }
        }
        fclose(fp);
    }

    if (android_api_level > 20)
        __system_property_set("debug.sf.fps", "1");
    else
        __system_property_set("debug.hwc.logfps", "1");
}

ChipBase::~ChipBase()
{
    if (android_api_level > 20)
        __system_property_set("debug.sf.fps", "0");
    else
        __system_property_set("debug.hwc.logfps", "0");
}

int ChipBase::get_model()
{
    int cpu_model = CHIP_UNSUPPORT;
    // get chip model
    if (__system_property_get("ro.board.platform", chip_model_str) && strlen(chip_model_str)>0) {
//        printf("ro.board.platform %s\n", value);
        if (strcasestr(chip_model_str, "3399"))
            cpu_model = CHIP_3399;
        else if (strcasestr(chip_model_str, "3368"))
            cpu_model = CHIP_3368;
        else if (strcasestr(chip_model_str, "3288"))
            cpu_model = CHIP_3288;
        else if (strcasestr(chip_model_str, "312"))
            cpu_model = CHIP_312x;
        else
            cpu_model = CHIP_UNSUPPORT;
    }

    return cpu_model;
}

int ChipBase::update_clk_summary()
{
    char line[256];
    FILE* fp = fopen("/sys/kernel/debug/clk/clk_summary", "r");
    if (fp == NULL)
        return -1;

    char clk_name[64];
    int enable_cnt;
    int prepare_cnt;
    unsigned int freq;
    int accuracy;
    int phase;

    clk_summary_idx = 0;
    while(fgets(line, sizeof(line), fp) && clk_summary_idx<1000) {
        if(sscanf(line, "%s %d %d %d %d %d", clk_name, &enable_cnt, &prepare_cnt, &freq, &accuracy, &phase)<4)
            continue;

        struct clk_info* p = &clk_summary[clk_summary_idx++];
        strcpy(p->clk_name, clk_name);
        p->enable_cnt = enable_cnt;
        p->prepare_cnt = prepare_cnt;
        p->freq = freq;
    }
/*
    int i=0;
    for(i=0; i<clk_summary_idx; i++) {
        printf("%d: %s, %d, %d, %d\n", i, clk_summary[i].clk_name, clk_summary[i].enable_cnt, clk_summary[i].prepare_cnt, clk_summary[i].freq);
    }
*/
    return 0;
}

int ChipBase::get_freq_from_summary(const char* name, int *enable_cnt)
{
    int freq_MHz = 0;
    int i=0;
    for(i=0; i<clk_summary_idx; i++) {
        if (strcmp(clk_summary[i].clk_name, name)==0) {
            freq_MHz = clk_summary[i].freq / 1000000;
            if(enable_cnt)
                *enable_cnt = clk_summary[i].enable_cnt;
            break;
        }
    }

    return freq_MHz;
}

int ChipBase::read_file_value(const char* file)
{
    int value = 0;
    char line[256];
    FILE* fp = fopen(file, "r");
    if (fp) {
        if (fgets(line, sizeof(line), fp)) {
            value = atoi(line);
        }
        fclose(fp);
    }

    return value;
}

int ChipBase::get_cpu_load(int* lit_cpu_load, int* big_cpu_load)
{
    FILE* fp = NULL;
    char line[256];
    int idx;
    long long user, nice, system, idle, iowait, irq, softirq;
    long long wake_cycle_b, total_cycle_b, wake_cycle_l, total_cycle_l;
    static long long prev_wake_cycle_b, prev_total_cycle_b, prev_wake_cycle_l, prev_total_cycle_l;
    wake_cycle_b = total_cycle_b = wake_cycle_l = total_cycle_l = 0;

    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
        return -1;

    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "cpu%d %lld %lld %lld %lld %lld %lld %lld", &idx, &user,
                        &nice, &system, &idle, &iowait, &irq, &softirq)<8)
            continue;

        long long used = user + nice + system + iowait + irq + softirq;
        long long total = used + idle;

        if (big_cpu_count && idx>=big_cpu_idx && idx<(big_cpu_idx+big_cpu_count)) {
            wake_cycle_b += used;
            total_cycle_b += total;
        }

        if (lit_cpu_count && idx>=lit_cpu_idx && idx<(lit_cpu_idx+lit_cpu_count)) {
            wake_cycle_l += used;
            total_cycle_l += total;
        }
    }
//    printf("cycle: %lld %lld %lld\n", total_cycle_b, total_cycle_l, total_cycle_l+total_cycle_b);

    if (big_cpu_count && big_cpu_load)
        *big_cpu_load = (wake_cycle_b-prev_wake_cycle_b)*100/(total_cycle_b-prev_total_cycle_b);
    prev_wake_cycle_b = wake_cycle_b;
    prev_total_cycle_b = total_cycle_b;

    if (lit_cpu_count && lit_cpu_load)
        *lit_cpu_load = (wake_cycle_l-prev_wake_cycle_l)*100/(total_cycle_l-prev_total_cycle_l);
    prev_wake_cycle_l = wake_cycle_l;
    prev_total_cycle_l = total_cycle_l;

//    printf("load: %d %d\n", cpu_b_info.load, cpu_l_info.load);

    fclose(fp);

    return 0;
}

int ChipBase::get_fps()
{
    int fps = 0;
    static char last_log_date[64] = "1";
    char cmd[128];

    if (android_api_level > 20)
        sprintf(cmd, "logcat -b main -v time -s SurfaceFlinger -t '%s'", last_log_date);
    else
        sprintf(cmd, "logcat -b main -v time -d -s hwcomposer -d");
//    printf("cmd: %s\n", cmd);

    FILE *fp=NULL;
    char buff[1024];
    memset(buff,0,sizeof(buff));
    if(NULL==(fp=popen(cmd, "r")))
    {
        fprintf(stderr,"execute command failed: %s",strerror(errno));
        return fps;
    }

    while(fgets(buff, sizeof(buff), fp))
    {
        char* p = strstr(buff, "mFps");
        if(p) {
            fps = atoi(p+6);
        }
    }
    pclose(fp);

    if (android_api_level > 20) {
        time_t timep;
        struct tm *p;
        time(&timep);
        p=localtime(&timep);
        sprintf(last_log_date, "%02d-%02d %02d:%02d:%02d.000", (1+p->tm_mon),
                    p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
    //    printf("update last_log_date: %s\n", last_log_date);
    } else {
        system("logcat -c");
    }

    return fps;
}

int ChipBase::set_cpu_freq(int core_idx, int freq_MHz)
{
    char governor_str[128];
    char setspeed_str[128];

    sprintf(governor_str, "/sys/bus/cpu/devices/cpu%d/cpufreq/scaling_governor", core_idx);
    sprintf(setspeed_str, "/sys/bus/cpu/devices/cpu%d/cpufreq/scaling_setspeed", core_idx);

    FILE* fp = fopen(governor_str, "r+");
    if (fp != NULL) {
        fwrite("userspace", 1, 9, fp);
        fclose(fp);

        fp = fopen(governor_str, "r+");
        if (fp != NULL) {
            char freq_str[256];
            sprintf(freq_str, "%d", freq_MHz*1000);
            fwrite(freq_str, 1, strlen(freq_str), fp);
            fclose(fp);
        }
    }

    return 0;
}

int ChipBase::get_ddr_load()
{
    int load = 0;
    if (linux_ver<=LINUX_3_10)
        load = 0;
    else
        load = read_file_value("/sys/devices/platform/dmc/devfreq/dmc/load");
    return load;
}

int ChipBase::get_vpu_freq()
{
    int freq, enable_cnt;
    freq = get_freq_from_summary(linux_ver==LINUX_3_10?"clk_vdpu":"aclk_vdpu", &enable_cnt);
    if (enable_cnt <= 0) {
        freq = get_freq_from_summary(linux_ver==LINUX_3_10?"clk_hevc_core":"sclk_hevc_core", &enable_cnt);
        if (enable_cnt <= 0)
            freq = 0;
    }

    return freq;
}