//
// Created by user1 on 10/4/23.
//

#ifndef MNGINX_CONFIG_H
#define MNGINX_CONFIG_H

#include <stdio.h>

/// \b location 匹配规则
typedef enum rule_enum {
    RULE_DEFAULT,    // .
    RULE_EXACT,      // =
    RULE_PREFIX,     // ~^
    RULE_REGEX       // ~
} rule_type;
/// \b item 用于存储 proxy_set_header 的 key 和 value
typedef struct item_struct {
    char *key;
    char *value;
} item;
/// \b location 用于存储 location 配置
/// \param rule 匹配规则 TODO 使用规则进行匹配
typedef struct location_struct {
    rule_type rule;
    char *pattern;
    int is_static;
    // dynamic
    item *proxy_set_header; // several headers, e.g. HOST
    char *proxy_pass_host;
    int proxy_pass_port;
// static
    char *root;
    char *index;
    struct location_struct *next;// use link table to store locations
} location;
/// \b server 用于存储 server 配置
typedef struct server_struct {
    /* read from conf */
    int listen;
    char *server_name;
    char *error_log;
    char *access_log;
    location *first_loc; // several locations
    /* manually added */
    struct server_struct *next;// use link table to store servers
    FILE *fe;// error log file ptr
    FILE *fa;// access log file ptr
} server;

/* global var */
extern server *server_head;

/*  func declaration   */
char *StrStrip(char *s);

void read_in_conf();

int mkdir_rec_no_file(const char *path);

void parse_url_host_port(char *url, char **s_name, int *port);
/* MACRO  */
// hash function mod
#define H_MOD  41
// hash result of given key
#define H_server 4
#define H_location 1
// server
#define H_listen 14
#define H_server_name 2
#define H_error_log 24
#define H_access_log 12
// location
#define H_rule 32 // 匹配规
#define H_pattern 10
// location - dynamic
#define H_proxy_set_header 16
#define H_proxy_pass 7
// location - static
#define H_root 15
#define H_index 34

#endif //MNGINX_CONFIG_H
