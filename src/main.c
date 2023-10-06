
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include "data_trans.h"
#include "header_edit.h"
#include "http_response.h"
#include "config.h"
#include "proxy.h"
#include "log.h"
#define MAX_THREADS 100
int main(void) {
    init_log();
    read_in_conf();
    // bind & listen; mn as a server
    pthread_t threads[MAX_THREADS];
    int num_threads = 0;
    for (server *server_conf = server_head->next; server_conf != NULL; server_conf = server_conf->next) {
        pthread_create(&threads[num_threads], NULL, (void *) main_process, (void *) server_conf);
        num_threads++;
        if (num_threads >= MAX_THREADS) {
            // 达到最大线程数，等待所有线程结束
            for (int i = 0; i < num_threads; i++) {
                pthread_join(threads[i], NULL);
            }
            num_threads = 0;
        }
    }
    // 等待剩余线程结束
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

//    main_process(server_head->next);
}

