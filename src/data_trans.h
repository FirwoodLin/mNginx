//
// Created by user1 on 10/3/23.
//

#ifndef MNGINX_DATA_TRANS_H
#define MNGINX_DATA_TRANS_H

#include <sys/socket.h>

//int end_with_dual_crlf(const char *data);
int end_with_dual_crlf(const char *data, size_t len);
void client_to_mn(int, char **);

void mn_to_client(int, const char *, ssize_t);

void mn_to_server(int, const char *);

void server_to_mn(int, char **);

#endif //MNGINX_DATA_TRANS_H
