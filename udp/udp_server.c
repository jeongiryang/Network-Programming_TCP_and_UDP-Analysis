#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 2022
#define BUFSIZE 4096

void measure(int server) {
    char buffer[BUFSIZE];
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    int total = 0;
    time_t start = time(NULL);
    ssize_t len;

    for (;;) {
        clilen = sizeof(cliaddr);
        len = recvfrom(server, buffer, BUFSIZE - 1, 0, (struct sockaddr*)&cliaddr, &clilen);
        if (len > 0) {
            buffer[len] = '\0';
            total += len;
            printf("수신 데이터: %s\n", buffer);

            time_t now = time(NULL);
            if (now - start >= 1) {
                printf("UDP Throughput: %d bytes/sec\n", total);
                total = 0;
                start = now;
            }
        }
    }
}

int main() {
    int server;
    struct sockaddr_in servaddr;

    server = socket(AF_INET, SOCK_DGRAM, 0);
    if (server < 0) exit(1);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) exit(1);

    printf("포트 %d 대기 중\n", PORT);
    measure(server);

    close(server);
    return 0;
}