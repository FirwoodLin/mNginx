//
// Created by user1 on 10/5/23.
//

#include "util.h"
#include "log.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// hash function
unsigned int BKDRHash(char *str) {
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str) {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

// allocate memory for dest, and copy src to dest
void alloc_cpy(char **dest, char *src) {
//    printf("alloc_cpy dest:%p,src:%s\n", *dest, src);
    size_t len = strlen(src) + 1;
    *dest = (char *) malloc(len);
    memset(*dest, 0x00, len);
    strcpy(*dest, src);
}

/// \b 打开前检查文件存在和权限
/// \param path 文件路径
/// \param flag 权限
/// \return 文件指针, NULL 表示打开失败
FILE *open_file(const char *path, int flag, const char *mode) {
    if (access(path, F_OK) == -1) {
        log_error(NULL, NULL, "file %s not exist", path);
        return NULL;
    }
    if (access(path, flag) == -1) {
        log_error(NULL, NULL, "file %s: permission denied", path);
        return NULL;
    }
    FILE *f = fopen(path, mode);
    if (f == NULL) {
        log_error(NULL, NULL, "file %s: open failed", path);
        return NULL;
    }
    return f;
}