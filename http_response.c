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

void http_not_found(int fd) {

    char *ret = (char *) malloc(strlen(headers_file[TYPE_NOT_FOUND]) + 20);
    memset(ret, 0x00, strlen(headers_file[TYPE_NOT_FOUND]) + 20);
    char *date = NULL;
    get_time(&date);
    sprintf(ret, headers_file[TYPE_NOT_FOUND], date);
    write(fd, ret, strlen(ret));
}