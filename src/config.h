//
// Created by user1 on 10/4/23.
//

#ifndef MNGINX_CONFIG_H
#define MNGINX_CONFIG_H
// location rule
typedef enum {
    RULE_DEFAULT,    // .
    RULE_EXACT,      // =
    RULE_PREFIX,     // ~^
    RULE_REGEX       // ~
} rule_type;
typedef struct item_struct {
    char *key;
    char *value;
} item;
typedef struct location_struct {
    rule_type rule;
    char *pattern;
    // dynamic
    item *proxy_set_header; // several headers, e.g. HOST
    char *proxy_pass_host;
    int proxy_pass_port;
// static
    char *root;
    char *index;
    struct location_struct *next;// use link table to store locations
} location;

typedef struct server_struct {
    int listen;
    char *server_name;
    char *error_log;
    char *access_log;
    location *first_loc; // several locations
    struct server_struct *next;// use link table to store servers
} server;
// global var
extern server *server_head;

// func
char *StrStrip(char *s);

void read_in_conf();

void alloc_cpy(char *dest, char *src);

unsigned int BKDRHash(char *str);

// const var
// hash result of given key
// hash function mod
const int H_MOD = 41;
const int H_server = 4;
const int H_location = 1;
// server
const int H_listen = 14;
const int H_server_name = 2;
const int H_error_log = 24;
const int H_access_log = 12;
// location
const int H_rule = 32;  // 匹配规则
const int H_pattern = 10;
// location - dynamic
const int H_proxy_set_header = 16;
const int H_proxy_pass = 7;
// location - static
const int H_root = 15;
const int H_index = 34;
#endif //MNGINX_CONFIG_H
