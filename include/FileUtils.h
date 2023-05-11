#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include <string.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <unistd.h>

#ifndef MAX_PATH
#define MAX_PATH    1024
#endif

class Node
{
public:
    Node(const char*a, const char* b=NULL, float c=1) {
        if (a)
            strcpy(node, a);
        else
            node[0] = 0;

        if (b)
            strcpy(node, b);
        else
            key[0] = 0;

        factor=c;
    }

    char node[256];
    char key[32];
    float factor;
};

int ReadFileAll(const char* filename, char* content, int len);
int ReadFileValue(const char* filename, uint64_t *result);
int ReadFileValue(Node& node, uint64_t *result);
int ReadFListValue(std::vector<Node>& filelist, uint64_t *result);
int WriteFileAll(const char* filename, const char* content);

#endif