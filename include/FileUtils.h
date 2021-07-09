#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include <string.h>
#include <string>
#include <vector>

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
int ReadFileValue(const char* filename, int *result);
int ReadFileValue(Node& node, int *result);
int ReadFListValue(std::vector<Node>& filelist, int *result);

#endif