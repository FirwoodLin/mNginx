//
// Created by user1 on 10/2/23.
//

#ifndef MNGINX_HEADER_EDIT_H
#define MNGINX_HEADER_EDIT_H

typedef enum {
    TYPE_HTML,
    TYPE_TXT,
    TYPE_PNG,
    TYPE_NOT_FOUND
} header_type;

void strrpc(char **str, char *oldstr, char *newstr);

void get_time(char **data); // get current time, store in data
void mTime(char *filepath, char **data);

header_type get_mime_type(char *filepath, char **data);//get the mime type of the file,store in data
char const *get_header_formatter(char *);

extern const char *headers_file[];
#endif //MNGINX_HEADER_EDIT_H