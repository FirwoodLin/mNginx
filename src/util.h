//
// Created by user1 on 10/5/23.
//

#ifndef MNGINX_UTIL_H
#define MNGINX_UTIL_H

#include "log.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

FILE *open_file(const char *path, int flag, const char *mode);

unsigned int BKDRHash(char *str);

void alloc_cpy(char **dest, char *src);

int mkdir_rec(const char *path);
#endif //MNGINX_UTIL_H
