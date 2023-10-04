//
// Created by user1 on 10/3/23.
//
#include "data_trans.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void client_to_mn(int fd, char **received_data) {
    // note: use char ** instead of char *
    // TODO: what if the data pack is larger than 1024 byte?
    // TODO: put multipart data together
//    for(;;) {
    printf("begin fd:%d\n", fd);
    char buf[1024];
    memset(buf, 0x00, sizeof(buf));
    ssize_t ret = read(fd, buf, 1023);
    *received_data = (char *) malloc(1024);
    strcpy(*received_data, buf);
    if (ret == 0) {
        return;
//            break;
    }
    if (ret == -1) {
        perror("read");
        return;
    }
    printf("buf size:%zd\n", ret);
    printf("%s\n", buf);
//    }
}

void mn_to_server(int fd, const char *data) {
    // 发送数据
    ssize_t num_bytes_sent = send(fd, data, strlen(data), 0);
    if (num_bytes_sent == -1) {
        perror("sendto");
        exit(1);
    }
    printf("Sent %zd bytes to the server.\n", num_bytes_sent);
}

void mn_to_client(int fd, const char *data, ssize_t n) {
    // n is the length of data
    printf("mn_to_client;data to write:\n%s\n", data);
    ssize_t ret;
    ret = write(fd, data, n);
    if (ret < 0) {
        printf("mn_to_client;write error\n");
    }
}


void server_to_mn(int fd, char **data) {
    char buff[1024];
    memset(buff, 0x00, sizeof buff);
    if (recv(fd, buff, 1024, 0) < 0) {
        printf("Error while receiving server's msg\n");
        exit(-1);
    }
    *data = (char *) malloc(strlen(buff));
    strcpy(*data, buff);
}