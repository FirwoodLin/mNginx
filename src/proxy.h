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
    int port;
    char *server_name;
} request;

void main_process(server *server_conf);

void process_data(char **);

int is_static_request(char *);

//void static_file(int, char **, char *);
void static_file(int fd, location *m_loc, request *req);

char *read_file(char *, long *);

location *find_best_match_location(request *req, server *server_conf);

request *parse_target(char *client_msg);

void parse_url(char *req_url, char **req_server_name, char **req_loc);

#endif //MNGINX_PROXY_H
