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
#include <sys/stat.h>

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
//    if (access(path, F_OK) == -1) {
//        log_error(NULL, NULL, "file %s not exist", path);
//        return NULL;
//    }
//    if (access(path, flag) == -1) {
//        log_error(NULL, NULL, "file %s: permission denied", path);
//        return NULL;
//    }
    FILE *f = fopen(path, mode);
    if (f == NULL) {
        log_error(NULL, NULL, "file %s: open failed", path);
        return NULL;
    }
    return f;
}

/// \b 递归创建目录
int mkdir_rec(const char *path) {
    char tmp[256];
    char *p = NULL;
    struct stat info;

    strncpy(tmp, path, sizeof(tmp));

    // 逐级检查并创建目录
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            // 检查目录是否存在
            if (stat(tmp, &info) != 0) {
//            if (stat(tmp, &info) != 0) {
                printf("正在创建目录 %s\n", tmp);
                // 目录不存在，创建目录
                mkdir(tmp, 0777);
            } else if (!S_ISDIR(info.st_mode)) {
                // 路径已经存在但不是文件夹
                printf("%s 不是一个目录\n", tmp);
                return -1;
            }

            *p = '/';
        }
    }
    // 创建最终文件夹
    if (stat(tmp, &info) != 0) {
        mkdir(tmp, 0777);
    } else if (!S_ISDIR(info.st_mode)) {
//        printf("%s is not a dir\n", tmp);
        log_error(DefaultCat, DefaultServer, "%s is not a dir", tmp);
        return -1;
    }
    return 0;
}
