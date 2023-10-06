//
// Created by user1 on 10/6/23.
//

#ifndef MNGINX_LOG_H
#define MNGINX_LOG_H

#include "config.h"

#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4

// 作为 error log 输出
#define log_warn(cat, ser, ...)                       \
    mlog(cat, ((server*)ser)->fe, ser, NULL, __FILE__, __func__, __LINE__, \
         LOG_LEVEL_WARN, __VA_ARGS__)
#define log_error(cat, ser, ...)                      \
    mlog(cat, ((server*)ser)->fe,ser, NULL, __FILE__, __func__, __LINE__, \
         LOG_LEVEL_WARN, __VA_ARGS__)
#define log_info(cat, ser, ...)                       \
    mlog(cat, ((server*)ser)->fe,ser, NULL, __FILE__, __func__, __LINE__, \
         LOG_LEVEL_INFO, __VA_ARGS__)
#define log_debug(cat, ser, ...)                      \
    mlog(cat, ((server*)ser)->fe,ser, NULL, __FILE__, __func__, __LINE__, \
         LOG_LEVEL_DEBUG, __VA_ARGS__)

// 作为access log输出 带有 sev, loc, 返回响应信息的 info 级别日志
#define log_info_e(cat, ser, loc, ...)               \
    mlog(cat, ((server*)ser)->fa, ser, loc, __FILE__, __func__, __LINE__, \
         LOG_LEVEL_INFO, __VA_ARGS__)
typedef struct {
    int level;
} category;
extern category *DefaultCat;
extern server *DefaultServer;
extern const char *LOG_LEVEL_MAP[];

void init_log();

void mlog(category *cat, FILE *f, server *ser, location *loc,
          const char *file, const char *func,
          long line, int level,
          const char *format, ...);

#endif //MNGINX_LOG_H
