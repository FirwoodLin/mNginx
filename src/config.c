//
// Created by user1 on 10/4/23.
//

#include "config.h"
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
    sprintf(formatString, "%%%d[^:]: %%%d[^\r\n]%%*2c", MAX_KV_LEN, MAX_KV_LEN);
    int flag_server_inputting = 0;
    int flag_location_inputting = 0;
    server *server_last = NULL;
    location *cur_location = NULL;// ptr to the location  that is inputting
    while ((ret = fscanf(f, formatString, a, b))) {
        if (ret == EOF) {
            printf("ret=%d\n,stop to read in conf \n", ret);
            break;
        }
        if (ret == 2) {
            StrStrip(a);
            unsigned int hash_val = BKDRHash(a) % H_MOD;
            switch (hash_val) {
                case H_server:
                    flag_server_inputting = 1;
                    flag_location_inputting = 0;
                    server *new_s = (server *) malloc(sizeof(server));
                    memset(new_s, 0x00, sizeof(server));
                    if (server_head == NULL) {
                        server_last = server_head->next = new_s;
                    } else {
                        server_last->next = new_s;
                        server_last = new_s;
                    }
                    memset(cur_location, 0x00, sizeof(location));
                    cur_location = NULL;
                    server_last->first_loc = cur_location;
                    break;
                case H_location:
                    flag_server_inputting = 0;
                    flag_location_inputting = 1;
                    location *last_location = cur_location;
                    cur_location = (location *) malloc(sizeof(location));
                    memset(cur_location, 0x00, sizeof(location));
                    if (last_location != NULL) {
                        last_location->next = cur_location;
                    }
                    break;
                default:
                    break;
            }
            if (flag_server_inputting) {
                switch (hash_val) {
                    case H_listen:
                        server_last->listen = atoi(b);
                        break;
                    case H_server_name:
                        alloc_cpy(server_last->server_name, b);

                        break;
                    case H_error_log:
                        alloc_cpy(server_last->error_log, b);
                    case H_access_log:
                        alloc_cpy(server_last->access_log, b);
                    default:
                        printf("server tag has no field\n");
                }
            } else if (flag_location_inputting) {
                switch (hash_val) {
                    case H_rule:
                        if (strcmp(b, ".") == 0) {
                            cur_location->rule = RULE_DEFAULT;
                        } else if (strcmp(b, "=") == 0) {
                            cur_location->rule = RULE_EXACT;
                        } else if (strcmp(b, "~^") == 0) {
                            cur_location->rule = RULE_PREFIX;
                        } else {
                            printf("invalid rule:%s\n", b);
                        }
                        break;
                    case H_pattern:
                        alloc_cpy(cur_location->pattern, b);
                        break;
                    case H_proxy_set_header:
                        // TODO:support multiple setting
                        cur_location->proxy_set_header = (item *) malloc(sizeof(item));
                        alloc_cpy(cur_location->proxy_set_header->key, strtok(b, " "));
                        alloc_cpy(cur_location->proxy_set_header->value, strtok(NULL, " "));
                        break;
                    case H_proxy_pass:
                        alloc_cpy(cur_location->proxy_pass_host, strtok(b, ":"));
                        cur_location->proxy_pass_port = atoi(strtok(NULL, ":"));
                        break;
                    case H_root:
                        alloc_cpy(cur_location->root, b);
                        break;
                    case H_index:
                        alloc_cpy(cur_location->index, b);
                        break;
                    default:
                        printf("location tag has no field\n");
                }
            }
        }
    }
    printf("conf read in finished!!!");
}

// hash function
unsigned int BKDRHash(char *str) {
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str) {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

void alloc_cpy(char *dest, char *src) {
    size_t len = strlen(src) + 1;
    dest = (char *) malloc(len);
    memset(dest, 0x00, len);
    strcpy(dest, src);
}