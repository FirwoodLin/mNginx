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


// const var
// hash result of given key
// hash function mod
#define H_MOD  41
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
