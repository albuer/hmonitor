#ifndef __RK3368_H__
#define __RK3368_H__

#include "ChipBase.h"

class RK3368Chip : public ChipBase {
public:
    RK3368Chip();
    virtual int get_cpu_freq_l();
    virtual int get_cpu_freq_b();
    virtual int get_cpu_temp();

    virtual int get_gpu_freq();
    virtual int get_gpu_load();
    virtual int get_gpu_temp();

    virtual int get_ddr_freq();

    virtual int get_vpu_freq();
};

#endif
