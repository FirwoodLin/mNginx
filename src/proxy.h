//
// Created by user1 on 10/4/23.
//

#ifndef MNGINX_PROXY_H
#define MNGINX_PROXY_H

#include "config.h"

#define H_MOD_REQUEST 23
#define H_Request_Header 15

#define H_Request_URL 7
#define H_Request_Method 0
#define H_Host 18
#define H_User_Agent 13
#define H_Accept 8

typedef struct request_header {
    char *request_method;
    char *request_url;
    char *host;
    char *user_agent;
    char *accept;
    unsigned port;
    char *server_name; // http:// 到 / 之间的部分，可能包含端口号
    char *location;
    int status_code;// 返回状态码
} request;
typedef struct hd_arg_struct {
    // handle_client arg
    int fd;
    server *server_conf;
    location *loc;
} hd_arg;

void *handle_client(void *arg);

void main_process(server *server_conf);

int replace_header(char **msg, char *key, char *val);

void process_header(char **, server *, location *);

void replace_server_name(char **msg, char *new_prefix);

int is_static_request(char *);

//void static_file(int, char **, char *);
void static_file(int fd, location *m_loc, request *req, server *server_conf);

char *read_file(char *, long *);

location *find_best_match_location(request *req, server *server_conf);

request *parse_target(const char *client_msg, size_t len);

void parse_url(char *req_url, char **req_server_name, char **req_loc);

#endif //MNGINX_PROXY_H
