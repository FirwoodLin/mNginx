#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void handle_client(int fd);

int main(void)
{
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1235);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    int server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == -1) {
        perror("cannot create socket");
        exit(1);
    }

    if (bind(server_fd, (struct sockaddr *)&sa, sizeof sa) == -1) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    for (;;) {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            perror("accept failed");
            close(server_fd);
            return 1;
        }

        handle_client(client_fd);

        if (shutdown(client_fd, SHUT_RDWR) == -1) {
            perror("shutdown failed");
        }
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

void handle_client(int fd){
    for(;;) {
        printf("begin fd:%d\n",fd);
        char buf[1024];
        memset(buf, 0x00, sizeof(buf));
        ssize_t ret = read(fd, buf, 1023);
        if(ret == 0) {
            return;
        }
        if(ret == -1) {
            perror("read");
            return;
        }
        printf("buf size:%zd",ret);
        printf("%s\n", buf);
    }
}