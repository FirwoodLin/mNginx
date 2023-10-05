//
// Created by user1 on 10/5/23.
//

#include "util.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

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