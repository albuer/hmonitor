#ifndef __RK3288_H__
#define __RK3288_H__

#include "ChipBase.h"

class RK3288Chip : public ChipBase {
public:
    RK3288Chip();
    virtual int get_cpu_rate_l();
    virtual int get_cpu_rate_b();
    virtual int get_cpu_temp();

    virtual int get_gpu_rate();
    virtual int get_gpu_load();
    virtual int get_gpu_temp();

    virtual int get_ddr_rate();
};

#endif
