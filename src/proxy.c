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
#include <pthread.h>

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
    log_info(DefaultCat, server_conf, "start to listen on port %d", server_conf->listen);
    // 接收请求
    for (;;) {
        // 在 server_fd 上接收请求, 生成 client_fd
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept failed");
            close(server_fd);
            exit(1);

        }
        // 多线程处理客户端请求
        pthread_t tid;
        hd_arg ha = {client_fd, server_conf};
        if (pthread_create(&tid, NULL, handle_client, &ha) != 0) {
            perror("pthread_create failed");
            close(client_fd);
            continue;
        }
        pthread_detach(tid);
    }
    close(server_fd);
}

void *handle_client(void *arg) {
    hd_arg *ha = (hd_arg *) arg;
    int client_fd = ha->fd;
    server *server_conf = ha->server_conf;
    log_debug(DefaultCat, server_conf, "start to handle_client");
    // 从 client_fd 中读取数据 生成 client_msg
    char *client_msg = NULL;    // data that comes from client
    size_t len = client_to_mn(client_fd, &client_msg);
    printf("client_to_mn finished\n");
    /*parse the client msg, get target loc*/
    request *req = parse_target(client_msg, len);
    /*find best match location, get the ptr to loc*/
    req->port = server_conf->listen;
    location *best_match_loc = find_best_match_location(req, server_conf);
    /*  process static request*/
    if (best_match_loc == NULL) {
        http_error(client_fd, TYPE_NOT_FOUND);
        close(client_fd);
        return NULL;
    }
    if (best_match_loc->is_static == 1) {
        static_file(client_fd, best_match_loc, req, server_conf);  // 生成响应报文 并返回
        close(client_fd);
        return NULL;
    }
    /*  process dynamic request*/
    process_header(&client_msg, server_conf, best_match_loc);
    // ===与服务器建立连接===
    char *proxy_pass_ip = best_match_loc->proxy_pass_host;
    int proxy_pass_port = best_match_loc->proxy_pass_port;
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 目标服务器的 fd
    if (fd == -1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(proxy_pass_ip);
    dest_addr.sin_port = htons(proxy_pass_port);
    if (connect(fd, (struct sockaddr *) &dest_addr, sizeof dest_addr) < 0) {
        perror("connect");
        exit(1);
    }
    printf("len msg to send to server: %zd\n", strlen(client_msg));
    // ===发送数据===
    mn_to_server(fd, client_msg, strlen(client_msg)); // send data via new socket
    char *server_msg = NULL;
    ssize_t ret_server = server_to_mn(fd, &server_msg);// receive data from socket
    if (ret_server < 0) {
//        printf("server_to_mn error\n");
//        close(fd);
        http_error(client_fd, TYPE_INTERNAL_ERROR);
        log_error(DefaultCat, server_conf, "server_to_mn error");
        close(client_fd);
        return NULL;
    }
    // send data via old socket
//    mn_to_client(client_fd, server_msg, ret_server);
    http_data_dynamic(client_fd, server_msg, ret_server);
    printf("finished a client_fd");
    if (shutdown(client_fd, SHUT_RDWR) == -1) {
        perror("shutdown failed");
    }
    return NULL;
}

/// \b 进行动态请求的头部信息替换
/// \param client_msg
/// \param server_conf
/// \param loc
void process_header(char **client_msg, server *server_conf, location *loc) {
    char *key = loc->proxy_set_header->key;
    char *val = loc->proxy_set_header->value;
    replace_header(client_msg, key, val); // 替换 host 信息
    replace_server_name(client_msg, loc->proxy_pass); //   替换 Request URL 信息
}

// Request URL 替换前缀为 proxy pass
void replace_server_name(char **msg, char *new_prefix) {
////    char *key = "Request URL: ";
//    char *msg_ptr = *msg;
////    char *key_pos = strstr(msg_ptr, key1);
//    char * key_pos = strstr(msg_ptr, "://");
//    key_pos += 3;
//    char *key_end_pos = strstr(key_pos, "/");//左闭右开
////    key_end_pos-=1;
//    char *val = (char *) malloc(key_end_pos - key_pos);
//    long  server_name_len = key_end_pos - key_pos;
//    memset(val, 0x00, server_name_len);
//    memcpy(val, key_pos, server_name_len);
//    char *key2 = "Request URL";
//    replace_header(msg, key2, val);
    char *msg_ptr = *msg;
    char key1[] = "Request URL: ";
    char *key_pos = strstr(msg_ptr, key1);  // 定位到所在行
    key_pos += strlen(key1);
    char *key_pos2 = strstr(key_pos, "://"); // 定位到协议起点
    key_pos2 += 3;
    char *key_pos3 = strstr(key_pos2, "/");  // 定位到域名终点
    char *key_pos4 = strstr(key_pos3, "\r\n"); // 定位到行终点
    long old_val_len = key_pos4 - key_pos;
    long new_val_len = strlen(new_prefix) + key_pos4 - key_pos3 + 1;
    char *new_val = (char *) malloc(new_val_len);
    memset(new_val, 0x00, new_val_len);
    strncpy(new_val, new_prefix, strlen(new_prefix));
    strncat(new_val, key_pos3, key_pos4 - key_pos3 - 1);
    replace_header(msg, "Request URL", new_val);

}

/// \brief 处理动态请求的头部数据，以 k-v 形式修改
/// \param client_msg
/// \param server_conf
/// \param loc
int replace_header(char **msg, char *key, char *val) {
    char *msg_ptr = *msg;
    char needle[strlen(key) + 3];
    strcpy(needle, key);
    strcat(needle, ": ");
    char *pos_key = strstr(msg_ptr, needle);
    if (pos_key == NULL) {
        printf("replace_header: key(%s) not found\n", key);
        return 1;
    }
    char *pos_val_end = strstr(pos_key, "\r\n");
    if (pos_val_end == NULL) {
        printf("replace_header: no val end found\n");
        return 1;
    }
    // 拼接新的报文
    size_t new_val_len = strlen(val);
    size_t msg_len = strlen(msg_ptr);
    size_t old_val_len = pos_val_end - pos_key - strlen(needle);
    size_t new_msg_len = msg_len + (new_val_len - old_val_len) + 1; // 计算替换后的字符串长度

    char *new_msg = (char *) malloc(new_msg_len);
    memset(new_msg, 0, new_msg_len);
    strncpy(new_msg, msg_ptr, pos_key - msg_ptr + strlen(needle));
    strcat(new_msg, val);
    strcat(new_msg, pos_val_end);
    *msg = new_msg;
    free(msg_ptr);
    return 0;
}

/// \brief 处理静态请求
/// \param fd
/// \param data
/// \param request
void static_file(int fd, location *m_loc, request *req, server *server_conf) {
    char *root = m_loc->root;
    char *index = m_loc->index;
    char *pattern = m_loc->pattern;
    char *req_loc = req->location;// loc 中有一部分是 pattern
    // 获取请求的文件的物理路径
    unsigned real_path_len = strlen(root) + strlen(req_loc) - strlen(pattern) + 1;
    char *real_path = (char *) malloc(real_path_len);
    memset(real_path, 0x00, real_path_len);
    strcpy(real_path, root);
    strcat(real_path, req_loc + strlen(pattern));
    // 读取文件
    long file_length = 0;// store the file length==content length
    char *file_content = read_file(real_path, &file_length);
    if (file_content == NULL) {
        // specific file not found,return index
        printf("static_file :%s not found,finding index file\n", real_path);
        char *buffer = read_file(index, &file_length);
        if (buffer == NULL) {
            printf("static_file: index file read error\n");
            http_error(fd, TYPE_NOT_FOUND);
            log_info_e(DefaultCat, server_conf, m_loc, "%s %d %s", req->request_url, 404, req->user_agent);
            return;
        }
        file_content = buffer;
    }
    // 收集首部信息
    char *header_mtime = NULL;// 修改时间
    get_modify_time(real_path, &header_mtime);// TODO: 时间年份不正确
    char *header_time = NULL;// 当前时间
    get_time(&header_time);
    char *header_content_type = NULL;// 文件类型
    get_mime_type(real_path, &header_content_type);
    const char *header_formatter = get_header_formatter(real_path);
    unsigned long header_length = (strlen(header_formatter) + strlen(header_mtime) + strlen(header_time) + 10);
    char *header = (char *) malloc(header_length);
    memset(header, 0x00, header_length);
//    // 组成报文首部
    sprintf(header, header_formatter, file_length, header_time, header_mtime);
    mn_to_client(fd, header, strlen(header));
//    // 分批发送数据
    mn_to_client(fd, file_content, file_length);
    log_info_e(DefaultCat, server_conf, m_loc, "%s %d %s", req->request_url, 200, req->user_agent);
}

/// \brief 读取文件，返回长度和内容
/// \param file_path 文件路径
/// \param file_length 存储文件长度
/// \return NULL if error，else return the file content
char *read_file(char *file_path, long *file_length) {
    char *buffer = NULL;
    FILE *f = fopen(file_path, "rb");
    if (f == NULL) {
        printf("read_file-file not found:%s\n", file_path);
        return NULL;
    }
    if (f) {
        // calc the file length
        fseek(f, 0, SEEK_END);
        long length = ftell(f);
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

request *parse_target(const char *client_msg, size_t len) {
//    char formatString[20]; // 用于存储动态构建的格式化字符串
//    sprintf(formatString, "%%%d[^:]: %%%d[^\r\n]%%*2c", MAX_KV_LEN, MAX_KV_LEN);
    printf("parse_target begin\n");
    request *req = (request *) malloc(sizeof(request));
    memset(req, 0x00, sizeof(request));
    char a[MAX_KV_LEN + 1], b[MAX_KV_LEN + 1];
    char *token;
    char *rest = (char *) malloc(len + 1);
    memset(rest, 0x00, len + 1);
    memcpy(rest, client_msg, len);  // 创造副本，防止修改原始数据
    token = strtok_r(rest, "\r\n", &rest);
    while (token != NULL && (sscanf(token, "%127[^:]: %127[^\r\n]%*2c", a, b) != EOF)) {
//        char *key = strtok(client_msg, ":");
        unsigned int key_hash = BKDRHash(a) % H_MOD_REQUEST;
        if (key_hash == H_Request_Header) {
            token = strtok_r(rest, "\r\n", &rest);

            continue;
        }
        printf("parse_target:%s#%s\n", a, b);
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
//    char *req_url = req->request_url;
//    req_server_name = req_loc = NULL;
    parse_url(req->request_url, &req->server_name, &req->location);
    // nginx 的 server_name 实际上是和请求中的 host 一致
    char *req_server_name = req->host, *req_loc = req->location;
    printf("find_best_match_location begin:req_server_name:%s,req_loc:%s\n", req_server_name, req_loc);
    for (server *server_ptr = server_conf; server_ptr != NULL; server_ptr = server_ptr->next) {
        if (strcmp(server_ptr->server_name, req_server_name) == 0 && server_ptr->listen == req->port) {
            // found the server; name and port must match
            printf("find:server_name:%s\n", server_ptr->server_name);
            for (location *loc_ptr = server_ptr->first_loc->next; loc_ptr != NULL; loc_ptr = loc_ptr->next) {
                if (strcmp(loc_ptr->pattern, req_loc) == 0) {
                    // found the location
//                    printf("find:loc:%s\n", loc_ptr->pattern);
                    log_debug(DefaultCat, server_ptr, "found location,pattern:%s;request pattern:%s", loc_ptr->pattern,
                              req_loc);
                    return loc_ptr;
                }
            }
        }
    }
    printf("best match not found:%s\n", req_loc);
    return NULL;
}