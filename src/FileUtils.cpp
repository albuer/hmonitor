#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "FileUtils.h"

/*
   返回值： 从文件中读取到的字节数； < 0 表示失败
 */
int ReadFileAll(const char* filename, char* content, int len)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
        return -1;

    int ret = read(fd, content, len);
    close(fd);

    return ret;
}

/*
   返回值： 从文件中读取到的字节数； < 0 表示失败
 */
int ReadFileValue(const char* filename, int *result)
{
    char content[128] = "";
    int ret = ReadFileAll(filename, content, sizeof(content)-1);

    if (ret >= 0) {
        *result = atoi(content);
    }

    return ret;
}

int ReadFileValue(Node& node, int *result)
{
    long long value = 0;
    char content[128] = "";
    int ret = ReadFileAll(node.node, content, sizeof(content)-1);

    if (ret >= 0) {
        if (node.key[0] != 0) {
            char*ptr = strstr(content, node.key);
            if (ptr != NULL) {
                ptr += strlen(node.key);
                value = atoll(content);
            }
        } else {
            value = atoll(content);
        }
    }

    *result = (int)(value / node.factor);

    return ret;
}

/*
   返回值：
     < 0  所有文件都不存在，或不可读
     >=0  存在的文件的序号
 */
int ReadFListValue(std::vector<Node>& filelist, int *result)
{
    int ret = -1;

    for (size_t i=0; i<filelist.size(); i++) {
        ret = ReadFileValue(filelist[i], result);
        if ( ret >= 0) {
            ret = i;
            break;
        }
    }

    return ret;
}