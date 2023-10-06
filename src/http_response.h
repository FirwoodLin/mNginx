//
// Created by user1 on 10/3/23.
//

#ifndef MNGINX_HTTP_RESPONSE_H
#define MNGINX_HTTP_RESPONSE_H

#include <unistd.h>

typedef enum {
    TYPE_HTML,
    TYPE_TXT,
    TYPE_PNG,
    TYPE_NOT_FOUND,
    TYPE_INTERNAL_ERROR,
    TYPE_BAD_REQUEST
} header_type;
extern const char *headers_file[];
#define HTTP_STATUS_NOT_FOUND 404
#define HTTP_STATUS_OK 200
#define HTTP_STATUS_BAD_REQUEST 400
#define HTTP_STATUS_INTERNAL_ERROR 500

void http_error(int fd, header_type ht); // 404 ,500, 400

void http_data_dynamic(int fd, const char *data, ssize_t len); //200

//void http_data_static(int fd, header_type ht, const char *data, ssize_t len); //200
void http_data_static(int fd, header_type ht, const char *data, ssize_t len, const char *file_path);

#endif //MNGINX_HTTP_RESPONSE_H
