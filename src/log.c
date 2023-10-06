//
// Created by user1 on 10/6/23.
//

#include "log.h"
#include "util.h"
#include "header_edit.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

const char *LOG_LEVEL_MAP[] = {
        "", "DEBUG", "INFO", "WARN", "ERROR"
};
// Default log param
category *DefaultCat;
server *DefaultServer;

void init_log() {
    DefaultCat = (category *) malloc(sizeof(category));
    DefaultCat->level = LOG_LEVEL_DEBUG;
    DefaultServer = (server *) malloc(sizeof(server));
    FILE *f = fopen("error.log", "a");
    if (f == NULL) {
        printf("open error.log failed\n");
        exit(1);
    }
    DefaultServer->fe = f;
    f = fopen("access.log", "a");
    if (f == NULL) {
        printf("open access.log failed\n");
        exit(1);
    }
    DefaultServer->fa = f;
}

void mlog(category *cat, FILE *f, server *ser, location *loc,
          const char *file, const char *func,
          long line, int level,
          const char *format, ...) {
//    if(loc!=NULL){
//
//    }
    if (cat->level > level) {
        return;
    }
    char *t = NULL;
    get_time(&t);
    // 时间 级别 文件 函数 行号
    fprintf(f, "[%s %s]%s %s-%ld", t, LOG_LEVEL_MAP[level], file, func, line);
//    free(t);
    if (loc != NULL) {
        // print context
        fprintf(f, "[server]listen:%d,servername:%s"
                   "[location]pattern:%s", ser->listen, ser->server_name, loc->pattern);
    }
    va_list args;
    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    fprintf(f, "\n");
    fflush(f);
}