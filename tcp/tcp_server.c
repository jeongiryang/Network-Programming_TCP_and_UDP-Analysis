#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 2022
#define BUFSIZE 4096

void measure(int client) {
    char buffer[BUFSIZE];
    int total = 0;
    time_t start = time(NULL);
    ssize_t len;

    while ((len = recv(client, buffer, BUFSIZE - 1, 0)) > 0) {
        buffer[len] = '\0';
        total += len;
        
        printf("수신 데이터: %s\n", buffer);

        time_t now = time(NULL);
        if (now - start >= 1) {
            printf("TCP Throughput: %d bytes/sec\n", total);
            total = 0;
            start = now;
        }
    }
}

int main() {
    int server, client;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) exit(1);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) exit(1);
    if (listen(server, 5) < 0) exit(1);

    printf("포트 %d 대기 중\n", PORT);

    for (;;) {
        clilen = sizeof(cliaddr);
        client = accept(server, (struct sockaddr*)&cliaddr, &clilen);
        if (client < 0) continue;
        
        printf("클라이언트 연결\n");
        measure(client);
        printf("연결 종료\n");
        close(client);
    }
    close(server);
    return 0;
}