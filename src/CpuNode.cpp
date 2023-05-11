#include "CpuNode.h"

CpuNode::CpuNode() : group_count_(0), temperature_(-1)
{
    int cpu_count = 0;
    char content[128];
    int begin, end;
    if (ReadFileAll("/sys/devices/system/cpu/present", content, sizeof(content)-1) < 0) {
        return;
    }
    sscanf(content, "%d-%d", &begin, &end);
    cpu_count = end-begin+1;

    // 统计cpu group 数量, 每个group的核数
    char filename[512];
    for (int i=0; i<cpu_count; i++) {
        sprintf(filename, "/sys/devices/system/cpu/cpufreq/policy%d", i);
        if (access(filename, F_OK) == 0) {
            if (i > 0) {
                groups_[group_count_].cores = i - groups_[group_count_].index;
                ++group_count_;
            }
            groups_[group_count_].index = i;
            groups_[group_count_].total_cycle = groups_[group_count_].wake_cycle = 0;
            groups_[group_count_].last_total_cycle = groups_[group_count_].last_wake_cycle = 0;
        }
    }
    groups_[group_count_].cores = cpu_count - groups_[group_count_].index;
    ++group_count_;

    // 查找CPU温度节点
    temperature_path_[0] = '\0';
    for (int i=0; i<10; i++) {
        sprintf(filename, "/sys/class/thermal/thermal_zone%d/type", i);
        if (ReadFileAll(filename, content, sizeof(content)-1) < 0) {
            continue;
        }
        if (strstr(content, "soc") != NULL) {
            sprintf(temperature_path_, "/sys/class/thermal/thermal_zone%d/temp", i);
            break;
        }
    }
}

int CpuNode::Update()
{
    if (group_count_ <= 0)
        return -1;

    UpdateFreq();
    UpdateLoad();
    UpdateTemp();

    return 0;
}

int CpuNode::UpdateLoad()
{
    FILE* fp = NULL;
    char line[256];
    int idx;
    long long user, nice, system, idle, iowait, irq, softirq;

    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
        return -1;

    for (int i=0; i<group_count_; i++) {
        groups_[i].last_wake_cycle = groups_[i].wake_cycle;
        groups_[i].last_total_cycle = groups_[i].total_cycle;
        groups_[i].wake_cycle = groups_[i].total_cycle = 0;
    }

    while(fgets(line, sizeof(line), fp)) {
        if(sscanf(line, "cpu%d %lld %lld %lld %lld %lld %lld %lld", &idx, &user,
                        &nice, &system, &idle, &iowait, &irq, &softirq)<8 ||
           line[3] == ' ')
            continue;

        struct CpuGroups* socinfo = NULL;
        for (int i=0; i<group_count_; i++) {
            if (idx >= groups_[i].index && idx < (groups_[i].index+groups_[i].cores)) {
                socinfo = &groups_[i];
                break;
            }
        }
        if (socinfo == NULL) {
            printf("cpu%d does not belong to any cpu group!\n", idx);
            continue;
        }

        long long used = user + nice + system + iowait + irq + softirq;
        socinfo->wake_cycle += used;
        socinfo->total_cycle += used + idle;
    }

    fclose(fp);

    for (int i=0; i<group_count_; i++) {
        // 第一次统计CPU负载
        if (groups_[i].last_wake_cycle == 0) {
            groups_[i].last_wake_cycle = groups_[i].wake_cycle;
            groups_[i].last_total_cycle = groups_[i].total_cycle;
        }
    }

    return 0;
}

int CpuNode::UpdateFreq()
{
    for (int i=0; i<group_count_; i++) {
        char filename[512];
        uint64_t freq;
        sprintf(filename, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_cur_freq", groups_[i].index);
        ReadFileValue(filename, &freq);
        groups_[i].freq = (int)(freq/1000); // MHz
    }

    return 0;
}

int CpuNode::UpdateTemp()
{
    uint64_t value = 0;
    if (temperature_path_[0] != 0 && ReadFileValue(temperature_path_, &value) >= 0) {
        temperature_ = (int)value;
    }

    return 0;
}

float CpuNode::GetLoad(int group)
{
    float wake_delta = groups_[group].wake_cycle - groups_[group].last_wake_cycle;
    float total_delta = groups_[group].total_cycle - groups_[group].last_total_cycle;

    return (total_delta>0) ? (wake_delta*100.0f/total_delta) : 0;
}

float CpuNode::GetLoadAvg()
{
    float wake_delta = 0;
    float total_delta = 0;
    for (int i=0; i<group_count_; i++) {
        wake_delta += groups_[i].wake_cycle - groups_[i].last_wake_cycle;
        total_delta += groups_[i].total_cycle - groups_[i].last_total_cycle;
    }

    return (total_delta>0) ? (wake_delta*100.0f/total_delta) : 0;
}

int CpuNode::GetFreqAvg()
{
    int freq = 0;
    int cores = 0;
    for (int i=0; i<group_count_; i++) {
        freq += groups_[i].freq * groups_[i].cores;
        cores += groups_[i].cores;
    }

    return (cores>0) ? (freq/cores) : 0;
}

int CpuNode::Boost()
{
    for (int i=0; i<group_count_; i++) {
        char filename[512];
        sprintf(filename, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", groups_[i].index);
        int ret = WriteFileAll(filename, "performance");
        printf("Boost CPU [%d-%d]: %s\n", groups_[i].index, groups_[i].index+groups_[i].cores-1, (ret>0)?"Success":"Failure");
    }

    return 0;
}