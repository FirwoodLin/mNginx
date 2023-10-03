//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "data_trans.h"
#include "header_edit.h"
#include "http_response.h"
void process_data(char **);

int is_static_request(char *);

void static_file(char **, char *);

char *read_file(char *, long *);

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
        /*========================*/
        /*  process static request*/
        if (is_static_request(client_msg)) {
            char *data = NULL;
            static_file(&data, client_msg);  // 生成响应报文 存储到 data
            if (data == NULL) {
                // 返回报错
                http_not_found(client_fd);
                continue;
            }
            mn_to_client(client_fd, data);// 将 data 返回
            printf("finished a client_fd");
            if (shutdown(client_fd, SHUT_RDWR) == -1) {
                perror("shutdown failed");
            }
            close(client_fd);
            continue;
        }
        /*========================*/
        process_data(&client_msg);
        // new socket between mn and server; mn as a client
        char proxy_pass_ip[100] = "127.0.0.1";// TODO: read in from conf
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


void process_data(char **client_msg) {
    char *data_ptr = *client_msg;
    char *data_backup = *client_msg;
    char *host = "$test.com";   //TODO: read in from conf
    char *needle = "Host: ";
    char *pos_host = strstr(*client_msg, needle);
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
    *client_msg = bstr;
    free(data_backup);
}

int is_static_request(char *request) {
    char *location = "/static";
    size_t head_len = strlen("GET ") + strlen(location);
    char *needle = (char *) malloc(head_len);
    strcpy(needle, "GET ");
    strcat(needle, location);
    char *pos = strstr(request, needle);
    if (pos == NULL) {
        printf("it is not a static file request\n");
        return 0;
    }
    return 1;
}

// data: store the file; request: pass the request
void static_file(char **data, char *request) {
    char location[] = "/static";
    char root[] = "./static";
    char index[] = "./static/index.html";    // default file
    size_t head_len = strlen("GET ") + strlen(location);
    char *needle = (char *) malloc(head_len);
    strcpy(needle, "GET ");
    strcat(needle, location);
    char *pos = strstr(request, needle);
    if (pos == NULL) {
        printf("it is not a static file request\n");
        return;
    }
    char *pos_end = strstr(pos, " HTTP");
    size_t file_path_len = pos_end - pos - head_len - 1;
    char *file_path = (char *) malloc(file_path_len);
    strncpy(file_path, pos + head_len + 1, file_path_len);
    long header_file_length = 0;// store the file length==content length
    char *target_file_content = read_file(file_path, &header_file_length);
    if (target_file_content == NULL) {
        // specific file not found,return index
        printf("file:%s not found\n", file_path);
        char *buffer = read_file(index, &header_file_length);
        if (buffer == NULL) {
            printf("read file error, index\n");
            return;
        }
        *data = buffer;
        return;
    }
    // return specific file
    *data = target_file_content;    // 获取了文件的内容，需要补充首部信息
    // 拼装首部信息
    char *header_mtime;// 当前时间
    mTime(file_path, &header_mtime);
    char *header_time;// 修改时间
    get_time(&header_time);
    char *header_content_type;// 文件类型
    get_mime_type(file_path, &header_content_type);
    const char *header_formatter = get_header_formatter(file_path);
    unsigned long header_length = (strlen(header_formatter) + strlen(header_mtime) + strlen(header_time) + 10);
    char *header = (char *) malloc(header_length);
    memset(header, 0x00, header_length);
    // 组成报文首部
    sprintf(header, header_formatter, header_file_length, header_time, header_mtime);
    // 组成完整报文
    char *response = (char *) malloc(header_length + header_file_length);
    sprintf(response, "%s%s", header, target_file_content);
    free(*data);
    *data = (char *) malloc(sizeof(response));
    strcpy(*data, response);
}

char *read_file(char *file_path, long *file_length) {
    char *buffer = NULL;
    long length;
    char real_path[100];
    memset(real_path, 0x00, sizeof(real_path));
    strcpy(real_path, "./static/");
    strcat(real_path, file_path);
    FILE *f = fopen(real_path, "r");
    if (f == NULL) {
        printf("read_file;file not found:%s\n", file_path);
    }
    if (f) {
        // calc the file length
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        *file_length = length;
        printf("calced file length:%ld\n", length);
        // return to beginning
        fseek(f, 0, SEEK_SET);
        buffer = (char *) malloc(1024);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        // fclose(f);
    }
    if (buffer == NULL) {
        printf("read_file-read file error\n");
        return NULL;
    }
    fclose(f);
    return buffer;
}