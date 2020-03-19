#ifndef __CHIP_BASE_H__
#define __CHIP_BASE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

enum CHIP_MODEL {
    CHIP_3399 = 0,
    CHIP_3368,
    CHIP_3288,
    CHIP_312x,
    CHIP_UNSUPPORT,
};

enum LINUX_VER {
    LINUX_3_10 = 0,
    LINUX_4_4,
    LINUX_UNKNOWN,
};

struct clk_info {
    char clk_name[64];
    int enable_cnt;
    int prepare_cnt;
    unsigned int rate;
};

class ChipBase {
public:
    static char chip_model_str[256];
    static struct clk_info* clk_summary;
    static int clk_summary_idx;
    int lit_cpu_idx;
    int lit_cpu_count;
    int big_cpu_idx;
    int big_cpu_count;
    int android_api_level;
    int linux_ver;
    char android_ver_str[256];
    char linux_ver_str[256];

    static int get_model();

    ChipBase();
    virtual ~ChipBase();

    int update_clk_summary();
    int get_cpu_load(int* lit_cpu_load, int* big_cpu_load);
    int get_fps();
    void set_cpu_rate_l(int rate_MHz) {set_cpu_rate(lit_cpu_idx, rate_MHz);};
    void set_cpu_rate_b(int rate_MHz) {set_cpu_rate(big_cpu_idx, rate_MHz);};

    virtual int get_cpu_rate_l() = 0;
    virtual int get_cpu_rate_b() = 0;
    virtual int get_cpu_temp() = 0;

    virtual int get_gpu_rate() = 0;
    virtual int get_gpu_load() = 0;
    virtual int get_gpu_temp() = 0;

    virtual int get_ddr_rate() = 0;
    int get_ddr_load();

    virtual int get_vpu_rate();

protected:
    int get_rate_from_summary(const char* name, int *enable_cnt);
    inline int get_rate_from_summary(const char* name){
        return get_rate_from_summary(name, (int*)0);
    };
    int read_file_value(const char* file);
    int set_cpu_rate(int core_idx, int rate_MHz);

private:
};

#endif
