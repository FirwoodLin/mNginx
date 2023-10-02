#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void client_to_mn(int, char **);

void mn_to_client(int, const char *);

void process_data(char **);

void mn_to_server(int, const char *);

void server_to_mn(int, char **);

int main(void) {
    // bind & listen; mn as a server
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1235);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == -1) {
        perror("cannot create socket");
        exit(1);
    }

    if (bind(server_fd, (struct sockaddr *) &sa, sizeof sa) == -1) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }
    // accept data
    for (;;) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            perror("accept failed");
            close(server_fd);
            return 1;
        }
        char *client_msg = NULL;    // data that comes from client
        client_to_mn(client_fd, &client_msg);
        process_data(&client_msg);
        // new socket between mn and server; mn as a client
        char proxy_pass_ip[100] = "127.0.0.1";
        int proxy_pass_port = 7000;
        int fd;
        struct sockaddr_in dest_addr;
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fd == -1) {
            perror("socket");
            exit(1);
        }
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(proxy_pass_port);
        dest_addr.sin_addr.s_addr = inet_addr(proxy_pass_ip);
        if (connect(fd, (struct sockaddr *) &dest_addr, sizeof dest_addr) < 0) {
            perror("connect");
            exit(1);
        }
        // data process via mn_server socket
        printf("msg to send to server:%s\n", client_msg);
        mn_to_server(fd, client_msg); // send data via new socket
        char *server_msg = NULL;
        server_to_mn(fd, &server_msg);// receive data from socket
        // send data via old socket
        mn_to_client(client_fd, server_msg);
        printf("finished a client_fd");
        if (shutdown(client_fd, SHUT_RDWR) == -1) {
            perror("shutdown failed");
        }
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

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

void mn_to_client(int fd, const char *data) {
    printf("mn_to_client;data to write:\n%s\n", data);
    ssize_t ret;
    ret = write(fd, data, strlen(data));
    if (ret < 0) {
        printf("mn_to_client;write error\n");
    }
}

void process_data(char **data) {
    // TODO: replace the host
    char *data_ptr = *data;
    char *data_backup = *data;
    char *host = "$test.com";   //TODO: read in from conf
    char *needle = "Host: ";
    char *pos_host = strstr(*data, needle);
    if (pos_host == NULL) {
        printf("process_data;no host found\n");
        return;
    }
    char *pos_host_end = strstr(pos_host, "\r\n");
    if (pos_host_end == NULL) {
        printf("process_data;no host end found\n");
        return;
    }
    size_t new_len = strlen(host);
    size_t str_len = strlen(data_ptr);
    size_t bstr_len = str_len + (new_len - (pos_host_end - pos_host - strlen(needle))) + 1; // 计算转换后的字符串长度
    char *bstr = (char *) malloc(bstr_len); // 动态分配转换缓冲区
    memset(bstr, 0, bstr_len);
    strncpy(bstr, data_ptr, pos_host - data_ptr);
    strcat(bstr, needle);
    strcat(bstr, host);
    strcat(bstr, pos_host_end);
    *data = bstr;
    free(data_backup);
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