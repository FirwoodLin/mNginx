//
// Created by user1 on 10/2/23.
//
#include "header_edit.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

typedef struct {
    char *extention;
    char *mime_type;
} mime_map;
mime_map mime_types[] = {
        ".html", "text/html",
        ".txt", "text/plain",
        ".png", "image/png",
};
const char *headers_file[] = {
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %d\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Date: %s\r\n"
        "Last-Modified: %s\r\n"
        "\r\n",
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %d\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Date: %s\r\n"
        "Last-Modified: %s\r\n"
        "\r\n",

        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %d\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Type: image/png\r\n"
        "Date: %s\r\n"
        "Last-Modified: %s\r\n"
        "\r\n",
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 19\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Date: %s\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "\r\n"
        "404 page not found\r\n"
};

/*strrpc replace the  oldstr in str with newstr*/
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
    free(*str);
    *str = bstr; // 转换成功，返回转换后的字符串
}

/*get_time get current UTC time*/
void get_time(char **data) {
    time_t now;
    struct tm *timeinfo;
    char buffer[80];
    time(&now);  // 获取当前时间
    timeinfo = gmtime(&now);  // 将当前时间转换为UTC时间
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    if (*data != NULL) free(*data);
    *data = (char *) malloc(sizeof(buffer));
    strcpy(*data, buffer);
}

/*get the modify time of the file, store in data*/
void mTime(char *filepath, char **data) {
    char buffer[80];
    // get the last modified time of the file
    struct stat attr;
    stat(filepath, &attr);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&(attr.st_mtime)));
//    printf("Last modified time: %s\n",buffer);
    *data = (char *) malloc(sizeof(buffer));
    strcpy(*data, buffer);
}

/*get the mimetype of the file*/
header_type get_mime_type(char *filepath, char **data) {
    char *extension = strrchr(filepath, '.');
    for (int i = 0; i < sizeof(mime_types) / sizeof(mime_map); i++) {
        if (strcmp(extension, mime_types[i].extention) == 0) {
            *data = (char *) malloc(sizeof mime_types[i].mime_type);
            strcpy(*data, mime_types[i].mime_type);
            return i;
        }
    }
    //default
    strcpy(*data, "text/plain");
    return TYPE_TXT;
}

/*get the corresponding header of file*/
char const *get_header_formatter(char *filepath) {
    char *extension = strrchr(filepath, '.');
    for (int i = 0; i < sizeof(mime_types) / sizeof(mime_map); i++) {
        if (strcmp(extension, mime_types[i].extention) == 0) {
            return headers_file[i];
        }
    }
    //default
    return headers_file[TYPE_NOT_FOUND];
}