//
// Created by user1 on 10/4/23.
//

#include "proxy.h"
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
#include "config.h"
#include "util.h"

#define MAX_KV_LEN 127

void main_process(server *server_conf) {
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(server_conf->listen);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    // 开始监听
    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == -1) {
        perror("cannot create socket");
        exit(1);
    }
    if (bind(server_fd, (struct sockaddr *) &sa, sizeof sa) == -1) {
        printf("cannot bind port %d\n", server_conf->listen);
        perror("bind failed");
        close(server_fd);
        exit(1);

    }
    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(1);
    }
    // 接收请求
    for (;;) {
        // 在 server_fd 上接收请求, 生成 client_fd
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept failed");
            close(server_fd);
            exit(1);

        }
        // 从 client_fd 中读取数据 生成 client_msg
        char *client_msg = NULL;    // data that comes from client
        client_to_mn(client_fd, &client_msg);
        printf("client_to_mn finished\n");
        /*parse the client msg, get target loc*/
        request *req = parse_target(client_msg);
        /*find best match location, get the ptr to loc*/
        location *best_match_loc = find_best_match_location(req, server_conf);
        /*  process static request*/
        if (is_static_request(client_msg)) {
            char *data = NULL;
            static_file(client_fd, best_match_loc, client_msg);  // 生成响应报文 并返回
            printf("finished a client_fd %d\n", client_fd);
            if (shutdown(client_fd, SHUT_RDWR) == -1) {
                perror("shutdown failed");
            }
            close(client_fd);
            continue;
        }
        /*  process dynamic request*/
        process_data(&client_msg);
        char *proxy_pass_ip = server_conf->first_loc->proxy_pass_host;// TODO: read in from conf
        int proxy_pass_port = server_conf->first_loc->proxy_pass_port;
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
        mn_to_client(client_fd, server_msg, strlen(server_msg));
        printf("finished a client_fd");
        if (shutdown(client_fd, SHUT_RDWR) == -1) {
            perror("shutdown failed");
        }
        close(client_fd);
    }
    close(server_fd);
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

/// \brief 处理静态请求
/// \param fd
/// \param data
/// \param request
void static_file(int fd, location *m_loc, request *req) {
//    char location[] = "/static";
//    char root[] = "./static";
//    char index[] = "./static/index.html";    // default file
    char *index = m_loc->index;
    char *req_loc = req->request_url
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
    char *file_path = (char *) malloc(file_path_len + 1);
    memset(file_path, 0x00, file_path_len + 1);
    strncpy(file_path, pos + head_len + 1, file_path_len);
    long header_file_length = 0;// store the file length==content length
    char *target_file_content = read_file(file_path, &header_file_length);
    if (target_file_content == NULL) {
        // specific file not found,return index
        printf("specific file:%s not found\n", file_path);
        char *buffer = read_file(index, &header_file_length);
        if (buffer == NULL) {
            printf("index file: read file error, \n");
            return;
        }
        *data = buffer;
        return;
    }
    // return specific file
    *data = target_file_content;    // 获取了文件的内容，需要补充首部信息
    // 收集首部信息
    char *header_mtime = NULL;// 当前时间
    mTime(file_path, &header_mtime);// TODO: 时间年份不正确
    char *header_time = NULL;// 修改时间
    get_time(&header_time);
    char *header_content_type = NULL;// 文件类型
    get_mime_type(file_path, &header_content_type);
    const char *header_formatter = get_header_formatter(file_path);
    unsigned long header_length = (strlen(header_formatter) + strlen(header_mtime) + strlen(header_time) + 10);
    char *header = (char *) malloc(header_length);
    memset(header, 0x00, header_length);
    // 组成报文首部
    sprintf(header, header_formatter, header_file_length, header_time, header_mtime);
    mn_to_client(fd, header, strlen(header));
    // 分批发送数据
    mn_to_client(fd, target_file_content, header_file_length);
}

/// \brief 读取文件，返回长度和内容
/// \param file_path 文件路径
/// \param file_length 存储文件长度
/// \return NULL if error，else return the file content
char *read_file(char *file_path, long *file_length) {
    char *buffer = NULL;
    long length;
    char real_path[100];
    memset(real_path, 0x00, sizeof(real_path));
    strcpy(real_path, "./static/");
    strcat(real_path, file_path);
    FILE *f = fopen(real_path, "rb");
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
        buffer = (char *) malloc(length);
        memset(buffer, 0x00, length);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
    }
    if (buffer == NULL) {
        printf("read_file-read file error\n");
        fclose(f);

        return NULL;
    }
    fclose(f);
    return buffer;
}

request *parse_target(char *client_msg) {
//    char formatString[20]; // 用于存储动态构建的格式化字符串
//    sprintf(formatString, "%%%d[^:]: %%%d[^\r\n]%%*2c", MAX_KV_LEN, MAX_KV_LEN);
    printf("parse_target begin\n");
    request *req = (request *) malloc(sizeof(request));
    memset(req, 0x00, sizeof(request));

    char a[MAX_KV_LEN + 1], b[MAX_KV_LEN + 1];
    char *token;
    char *rest = client_msg;
    token = strtok_r(rest, "\r\n", &rest);
    while (token != NULL && (sscanf(token, "%127[^:]: %127[^\r\n]%*2c", a, b) != EOF)) {
//        char *key = strtok(client_msg, ":");
        unsigned int key_hash = BKDRHash(a) % H_MOD_REQUEST;
        if (key_hash == H_Request_Header) {
            token = strtok_r(rest, "\r\n", &rest);

            continue;
        }
        printf("parse_target:%s#%s\n", a, b);
//        sscanf(client_msg, "%127s[^\r\n]%%*2c", b);
        switch (key_hash) {
            case H_Request_URL:
                alloc_cpy(&req->request_url, b);
                break;
            case H_Request_Method:
                alloc_cpy(&req->request_method, b);
                break;
            case H_Host:
                alloc_cpy(&req->host, b);
                break;
            case H_User_Agent:
                alloc_cpy(&req->user_agent, b);
                break;
            case H_Accept:
                alloc_cpy(&req->accept, b);
                break;
            default:
                printf("unknown:parse_target;key:%s,value:%s\n", a, b);
                break;
        }

        token = strtok_r(rest, "\r\n", &rest);

    }

    printf("parse_target finished;request_url:%s\n", req->request_url);
    return req;
}


void parse_url(char *req_url, char **req_server_name, char **req_loc) {
    char *pos = strstr(req_url, "://");
    if (pos == NULL) {
        printf("get_server_name;no protocol found\n");
        return;
    }
    char *pos_end = strstr(pos + 3, "/");
    if (pos_end == NULL) {
        printf("get_server_name;no path found\n");
        return;
    }
    size_t server_name_len = pos_end - pos - 3;// 3==://
    char *server_name = (char *) malloc(server_name_len + 1);
    memset(server_name, 0x00, server_name_len + 1);
    strncpy(server_name, pos + 3, server_name_len);
    *req_server_name = server_name;
    size_t loc_len = strlen(req_url) - (pos_end - req_url);
    char *loc = (char *) malloc(loc_len + 1);
    memset(loc, 0x00, loc_len + 1);
    strncpy(loc, pos_end, loc_len);
    *req_loc = loc;
}

location *find_best_match_location(request *req, server *server_conf) {
    for (server *server_ptr = server_conf; server_ptr != NULL; server_ptr = server_ptr->next) {
        if (strcmp(server_ptr->server_name, req->request_url))
    }
    return NULL;
}