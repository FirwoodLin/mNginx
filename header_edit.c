//
// Created by user1 on 10/2/23.
//
#include "header_edit.h"
#include <string.h>
#include <stdlib.h>

void strrpc(char **str, char *oldstr, char *newstr) {
    char *str_body = *str;
    size_t old_len = strlen(oldstr);
    size_t new_len = strlen(newstr);
    size_t str_len = strlen(str_body);
    size_t bstr_len = str_len + (new_len - old_len) + 1; // 计算转换后的字符串长度
    char *bstr = (char *) malloc(bstr_len); // 动态分配转换缓冲区
    memset(bstr, 0, bstr_len);

    size_t bstr_index = 0;
    for (size_t i = 0; i < str_len; i++) {
        if (!strncmp(str_body + i, oldstr, old_len)) { // 查找目标字符串
            strcat(bstr, newstr);
            bstr_index += new_len;
            i += old_len - 1;
        } else {
            bstr[bstr_index++] = str_body[i]; // 保存一字节进缓冲区
        }
    }

    bstr[bstr_index] = '\0'; // 添加字符串结尾的空字符

    char *t = *str;
    *str = bstr;
    free(t);
}