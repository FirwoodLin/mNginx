//
// Created by user1 on 10/2/23.
//

#ifndef MNGINX_HEADER_EDIT_H
#define MNGINX_HEADER_EDIT_H

#include "http_response.h"

//void strrpc(char **str, char *oldstr, char *newstr);

void get_time(char **data); // get current time, store in data

void get_modify_time(const char *filepath, char **data);

header_type get_mime_type(char *filepath, char **data);//get the mime type of the file,store in data

char const *get_header_formatter(char *);

#endif //MNGINX_HEADER_EDIT_H