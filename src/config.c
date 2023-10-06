//
// Created by user1 on 10/4/23.
//

#include "config.h"
#include "util.h"
#include "log.h"
#include "proxy.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_KV_LEN 128
server *server_head = NULL;

char *StrStrip(char *s) {
    size_t size;
    char *p1, *p2;
    size = strlen(s);
    if (!size)
        return s;
    p2 = s + size - 1;
    while ((p2 >= s) && isspace(*p2))
        p2--;
    *(p2 + 1) = '\0';
    p1 = s;
    while (*p1 && isspace(*p1))
        p1++;
    if (s != p1)
        memmove(s, p1, p2 - p1 + 2);
    return s;
}

void read_in_conf() {
    FILE *f = fopen("./conf/mnginx.conf", "r");
    // const int MAX_LEN=128;
    char a[MAX_KV_LEN], b[MAX_KV_LEN];
    int ret = 0;
    char formatString[20]; // 用于存储动态构建的格式化字符串
//    sprintf(formatString, "%%%d[^:]: %%%d[^\r\n]%%*2c%%*2c", MAX_KV_LEN, MAX_KV_LEN);
    sprintf(formatString, "%%%d[^:]: %%%d[^\r\n]", MAX_KV_LEN, MAX_KV_LEN);
    int flag_server_inputting = 0;
    int flag_location_inputting = 0;
    server *server_last = NULL;
    location *cur_location = NULL;// ptr to the location  that is inputting
    int line_count = 0;
    while ((ret = fscanf(f, formatString, a, b))) {
        if (ret == EOF) {
//            printf("ret=%d line cnt=%d,stop to read in conf \n", ret, line_count);
            log_info(DefaultCat, DefaultServer, "ret=%d line cnt=%d,stop to read in conf", ret, line_count);
            break;
        }
        StrStrip(a);
//        printf("line %d:%s#%s\n", line_count, a, b);
        line_count++;

        unsigned int hash_val = BKDRHash(a) % H_MOD;
//        if (ret == 1) {
        switch (hash_val) {
            case H_server: {
                flag_server_inputting = 1;
                flag_location_inputting = 0;
                server *new_s = (server *) malloc(sizeof(server));
                memset(new_s, 0x00, sizeof(server));
                if (server_head == NULL) {
                    server_head = (server *) malloc(sizeof(server));
                    memset(server_head, 0x00, sizeof(server));
                    server_last = server_head->next = new_s;
                } else {
                    server_last->next = new_s;
                    server_last = new_s;
                }
//                cur_location = NULL;
//                server_last->first_loc = cur_location;
//                cur_location = server_last->first_loc;
                server_last->first_loc = (location *) malloc(sizeof(location));
                memset(server_last->first_loc, 0x00, sizeof(location));
                cur_location = server_last->first_loc;
                break;
            }
            case H_location: {
                flag_server_inputting = 0;
                flag_location_inputting = 1;
                if (server_last->first_loc->next == NULL) {
                    // 现在是头节点状态
                    cur_location = (location *) malloc(sizeof(location));
                    memset(cur_location, 0x00, sizeof(location));
                    server_last->first_loc->next = cur_location;
                } else {
                    // 现在是中间节点状态
                    cur_location->next = (location *) malloc(sizeof(location));
                    memset(cur_location->next, 0x00, sizeof(location));
                    cur_location = cur_location->next;
                }
                break;
            }
            default:
                break;
        }
        if (flag_server_inputting && hash_val != H_server) {
            switch (hash_val) {
                case H_listen: {
                    int i_val = atoi(b);
                    server_last->listen = i_val;
                    log_debug(DefaultCat, DefaultServer, "conf-read:listen:%d", i_val);
                    break;
                }
                case H_server_name: {
                    alloc_cpy(&server_last->server_name, b);
                    log_debug(DefaultCat, DefaultServer, "conf-read:server_name:%s", b);
                    break;
                }
                case H_error_log: {
                    alloc_cpy(&server_last->error_log, b);
                    int ret_mk = mkdir_rec_no_file(b);
                    if (ret_mk == -1) {
                        log_error(DefaultCat, DefaultServer, "error_log dir %s: create failed", b);
                    }
                    FILE *fe = open_file(b, W_OK, "a+");
                    if (fe == NULL) {
                        log_error(DefaultCat, DefaultServer, "error_log file %s: open failed", b);
                    }
                    server_last->fe = fe;
                    log_debug(DefaultCat, DefaultServer, "conf-read:error_log:%s", b);
                    break;
                }

                case H_access_log: {
                    alloc_cpy(&server_last->access_log, b);
                    int ret_mk = mkdir_rec_no_file(b);
                    if (ret_mk == -1) {
                        log_error(DefaultCat, DefaultServer, "access_log file %s: mkdir failed", b);
                    }
                    FILE *fa = open_file(b, W_OK, "a+");
                    if (fa == NULL) {
                        log_error(DefaultCat, DefaultServer, "access_log file %s: open failed", b);
                    }
                    server_last->fa = fa;
                    log_debug(DefaultCat, DefaultServer, "conf-read:access_log:%s", b);
                    break;
                }
                default:
                    log_warn(DefaultCat, DefaultServer, "line %d server tag(%s) has no field\n", line_count, a);
                    break;
            }
        } else if (flag_location_inputting && hash_val != H_location) {
            switch (hash_val) {
                case H_rule: {
                    if (strcmp(b, ".") == 0) {
                        cur_location->rule = RULE_DEFAULT;
                    } else if (strcmp(b, "=") == 0) {
                        cur_location->rule = RULE_EXACT;
                    } else if (strcmp(b, "^~") == 0) {
                        cur_location->rule = RULE_PREFIX;
                    } else {
                        printf("line %d invalid rule:%s\n", line_count, b);
                    }
                    log_debug(DefaultCat, DefaultServer, "conf-read:rule:%s %d", b, cur_location->rule);
                    break;
                }
                case H_pattern: {
                    alloc_cpy(&cur_location->pattern, b);
                    log_debug(DefaultCat, DefaultServer, "conf-read:pattern:%s", b);
                    break;
                }
                case H_proxy_set_header: {  // TODO:support multiple setting
                    cur_location->proxy_set_header = (item *) malloc(sizeof(item));
                    alloc_cpy(&cur_location->proxy_set_header->key, strtok(b, " "));
                    alloc_cpy(&cur_location->proxy_set_header->value, strtok(NULL, " "));
                    log_debug(DefaultCat, DefaultServer, "conf-read:proxy_set_header:%s %s",
                              cur_location->proxy_set_header->key,
                              cur_location->proxy_set_header->value);
                    break;
                }
                case H_proxy_pass: {
//                    parse_url(b, &cur_location->proxy_pass_host, &cur_location->proxy_pass_port);
//                    alloc_cpy(&cur_location->proxy_pass_host, strtok(b, ":"));
//                    cur_location->proxy_pass_port = atoi(strtok(NULL, ":"));
                    alloc_cpy(&cur_location->proxy_pass, b);
                    parse_url_host_port(b, &cur_location->proxy_pass_host, &cur_location->proxy_pass_port);
                    log_debug(DefaultCat, DefaultServer, "conf-read:proxy_pass:%s %d",
                              cur_location->proxy_pass_host,
                              cur_location->proxy_pass_port);
                    break;
                }
                case H_root: {
                    alloc_cpy(&cur_location->root, b);
                    log_debug(DefaultCat, DefaultServer, "conf-read:root:%s", b);
                    cur_location->is_static = 1;
                    break;
                }
                case H_index: {
                    alloc_cpy(&cur_location->index, b);
                    log_debug(DefaultCat, DefaultServer, "conf-read:index:%s", b);
                    break;
                }
                default:
//                    printf("line %d location tag has no field\na:%s\n,b:%s\n", line_count, a, b);
                    log_warn(DefaultCat, DefaultServer, "line %d location tag(%s) has no field\na:%s\n,b:%s\n",
                             line_count, a, b);
            }
        }

    }
//    printf("conf read in finished!!!\n");
    log_info(DefaultCat, DefaultServer, "conf read in finished");
}

/// \b 递归创建目录，不创建文件
/// \param path 文件路径,包含文件名
int mkdir_rec_no_file(const char *path) {
    size_t length = strlen(path);
    char tmp[256];
    // 找到最后一个目录分隔符 '/'
    const char *last_slash = strrchr(path, '/');
    if (last_slash != NULL) {
        // 计算目录部分的长度
        size_t dir_length = last_slash - path;
        // 复制目录部分到目标字符串
        strncpy(tmp, path, dir_length);
        tmp[dir_length] = '\0';
    } else {
        // 如果没有目录分隔符，则目录部分为空
        tmp[0] = '\0';
    }
    return mkdir_rec(tmp);
}

/// \b 从形如 http://1.1.1.1:80/ 中解析出 server_name 和 port
void parse_url_host_port(char *url, char **s_name, int *port) {
    char *pos = strstr(url, "://");
    if (pos == NULL) {
        printf("get_server_name;no protocol found\n");
        return;
    }
    char *pos_end = strstr(pos + 3, ":");
    if (pos_end == NULL) {
        printf("get_server_name;no path found\n");
        return;
    }
    size_t server_name_len = pos_end - pos - 3;// 3==://
    char *server_name = (char *) malloc(server_name_len + 1);
    memset(server_name, 0x00, server_name_len + 1);
    strncpy(server_name, pos + 3, server_name_len);
    *s_name = server_name;
    // port
    size_t loc_len = strlen(url) - (pos_end - url) - 2;
    char *port_s = (char *) malloc(loc_len + 1);
    memset(port_s, 0x00, loc_len + 1);
    strncpy(port_s, pos_end + 1, loc_len);
//    *req_loc = port_s;
    *port = atoi(port_s);
//    free(port_s);
}
