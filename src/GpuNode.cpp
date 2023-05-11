#include "GpuNode.h"

GpuNode::GpuNode()
{
    DIR *dir;
    struct dirent *ptr;
    freq_ = load_ = temp_ = -1;
    freq_path_[0] = load_path_[0] = temp_path_[0] = governor_path_[0] = '\0';

    if ((dir=opendir("/sys/class/devfreq")) != NULL)
    {
        while ((ptr=readdir(dir)) != NULL) {
            if (strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)
                continue;
            if (ptr->d_type == DT_LNK && (strstr(ptr->d_name, ".gpu") != NULL || strstr(ptr->d_name, "g6110") != NULL)) {
                sprintf(freq_path_, "/sys/class/devfreq/%s/cur_freq", ptr->d_name);
                sprintf(load_path_, "/sys/class/devfreq/%s/load", ptr->d_name);
                sprintf(governor_path_, "/sys/class/devfreq/%s/governor", ptr->d_name);
                break;
            }
        }
    }

    // 查找GPU温度节点
    for (int i=0; i<10; i++) {
        char content[128];
        char filename[512];
        sprintf(filename, "/sys/class/thermal/thermal_zone%d/type", i);
        if (ReadFileAll(filename, content, sizeof(content)-1) < 0) {
            continue;
        }
        if (strstr(content, "gpu") != NULL) {
            sprintf(temp_path_, "/sys/class/thermal/thermal_zone%d/temp", i);
            break;
        }
    }
}

int GpuNode::Update()
{
    uint64_t value = 0;
    freq_ = load_ = temp_ = -1;

    if (freq_path_[0] != 0 && ReadFileValue(freq_path_, &value) >= 0) {
        freq_ = (int)(value/1000000); // MHz
    }

    if (load_path_[0] != 0 && ReadFileValue(load_path_, &value) >= 0) {
        load_ = (int)value;
    }

    if (temp_path_[0] != 0 && ReadFileValue(temp_path_, &value) >= 0) {
        temp_ = (int)value;
    }

    return 0;
}

int GpuNode::Boost()
{
    int ret = WriteFileAll(governor_path_, "performance");
    printf("Boost GPU: %s\n", (ret>0)?"Success":"Failure");

    return ret;
}