//
// Created by user1 on 10/3/23.
//
#include "data_trans.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//void client_to_mn(int fd, char **received_data) {
//    // note: use char ** instead of char *
//    // TODO: what if the data pack is larger than 1024 byte?
//    // TODO: put multipart data together
////    for(;;) {
//    printf("begin fd:%d\n", fd);
//    char buf[1024];
//    memset(buf, 0x00, sizeof(buf));
//    ssize_t ret = read(fd, buf, 1023);
//    *received_data = (char *) malloc(ret);
//    memcpy(*received_data, buf, ret);
//    if (ret == 0) {
//        return;
////            break;
//    }
//    if (ret == -1) {
//        perror("read");
//        return;
//    }
//    printf("buf size:%zd\n", ret);
//    printf("%s\n", buf);
////    }
//}
void client_to_mn(int fd, char **received_data) {
    // 初始化动态缓冲区
    size_t buffer_size = 1024;  // 初始缓冲区大小
    size_t data_length = 0;     // 已接收数据的长度
    *received_data = (char *) malloc(buffer_size);
    if (*received_data == NULL) {
        perror("malloc");
        return;
    }
    // 接收数据
    printf("client_to_mn begin fd:%d\n", fd);
    for (;;) {
        char buf[1024];
        ssize_t ret = read(fd, buf, sizeof(buf));
        if (ret == 0) {
            printf("client_to_mn end fd:%d ret=0\n", fd);
            return;
        }
        if (ret == -1) {
            perror("read");
            return;
        }
        // 检查是否需要扩展缓冲区大小
        if (data_length + ret > buffer_size) {
            buffer_size *= 2;  // 扩展为当前大小的两倍
            char *new_buffer = (char *) realloc(*received_data, buffer_size);
            if (new_buffer == NULL) {
                perror("realloc");
                free(*received_data);
                *received_data = NULL;
                return;
            }
            *received_data = new_buffer;
        }
        // 将读取的数据拷贝到缓冲区
        memcpy(*received_data + data_length, buf, ret);
        data_length += ret;
        if (end_with_dual_crlf(buf, ret)) {
            printf("fd %d接收数据总长:%zd\n", fd, data_length);
            break;  // 读取完毕，退出循环
        }
    }

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

int end_with_dual_crlf(const char *data, size_t len) {
//    size_t len = strlen(data);
    if (len < 4) {
        return 0;
    }
    if (data[len - 1] == '\n' && data[len - 2] == '\r' && data[len - 3] == '\n' && data[len - 4] == '\r') {
        return 1;
    }
    return 0;
}