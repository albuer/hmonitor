#ifndef __RK312X_H__
#define __RK312X_H__

#include "ChipBase.h"

class RK312xChip : public ChipBase {
public:
    RK312xChip();
    virtual int get_cpu_rate_l();
    virtual int get_cpu_rate_b();
    virtual int get_cpu_temp();

    virtual int get_gpu_rate();
    virtual int get_gpu_load();
    virtual int get_gpu_temp();

    virtual int get_ddr_rate();
    virtual int get_ddr_load();

    virtual int get_vcodec_rate();
    virtual int get_hevc_rate();
};

#endif
