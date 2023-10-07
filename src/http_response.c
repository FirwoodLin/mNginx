//
// Created by user1 on 10/3/23.
//

#include <unistd.h>
#include "http_response.h"
#include "header_edit.h"
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char *headers_file[] = {
        // text
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %d\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Date: %s\r\n"
        "Last-Modified: %s\r\n"
        "\r\n",
        // html
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %d\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Date: %s\r\n"
        "Last-Modified: %s\r\n"
        "\r\n",
        // png
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: %d\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Type: image/png\r\n"
        "Date: %s\r\n"
        "Last-Modified: %s\r\n"
        "\r\n",
        // text not found
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 19\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Date: %s\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "\r\n"
        "404 page not found\r\n",
        // internal error
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Content-Length: 27\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Date: %s\r\n"
        "\r\n"
        "500 internal server error\r\n",
        // bad request
        "HTTP/1.1 400 Bad Request\r\n"
        "Content-Length: 17\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Date: %s\r\n"
        "\r\n"
        "400 bad request\r\n",
};

// not found, internal error, bad request
void http_error(int fd, header_type ht) {
    char *ret = (char *) malloc(strlen(headers_file[ht]) + 20);
    memset(ret, 0x00, strlen(headers_file[ht]) + 20);
    char *date = NULL;
    get_time(&date);
    sprintf(ret, headers_file[ht], date);
    write(fd, ret, strlen(ret));
}

void http_data_dynamic(int fd, const char *data, ssize_t len) {
//    log_info
    char *edited_header = (char *) malloc(strlen(headers_file[TYPE_TXT]) + 20);
    memset(edited_header, 0x00, strlen(headers_file[TYPE_TXT]) + 20);
    char *date_s = NULL;
    get_time(&date_s);
    sprintf(edited_header, headers_file[TYPE_TXT], len, date_s, date_s);
    write(fd, edited_header, strlen(edited_header));
    write(fd, data, len);
}

void http_data_static(int fd, header_type ht, const char *data, ssize_t len, const char *file_path) {
    char *edited_header = (char *) malloc(strlen(headers_file[ht]) + 20);
    memset(edited_header, 0x00, strlen(headers_file[ht]) + 20);
    char *date_s = NULL;
    char *date_m = NULL;
    get_time(&date_s);
    get_modify_time(file_path, &date_m);
    sprintf(edited_header, headers_file[ht], len, date_s, date_m);
    write(fd, edited_header, strlen(edited_header));
    write(fd, data, len);
}