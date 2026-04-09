#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 2022
#define BUFSIZE 2017

void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

void measure(int client, int showData) {
    char buffer[BUFSIZE];
    int totalBytes = 0;
    time_t startTime = time(NULL);
    ssize_t readLen;
    char sampleData[101] = {0};

    while ((readLen = recv(client, buffer, BUFSIZE - 1, 0)) > 0) {
        buffer[readLen] = '\0';
        totalBytes += readLen;
        
        if (showData) {
            strncpy(sampleData, buffer, 100); 
        }

        time_t currentTime = time(NULL);
        if (currentTime - startTime >= 1) {
            if (showData) {
                printf("수신 데이터 샘플: %s\n", sampleData);
            }
            printf("TCP Throughput: %d bytes/sec\n", totalBytes);
            printf("------------------------------------\n");
            
            totalBytes = 0;
            startTime = currentTime;
        }
    }
}

int main() {
    int serverSock, clientSock, showData;
    struct sockaddr_in servAddr, cliAddr;
    socklen_t cliLen;

    printf("데이터 출력? (1: 예, 0: 아니오): ");
    if (scanf("%d", &showData) != 1) showData = 0;

    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) handleError("socket error");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    if (bind(serverSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) 
        handleError("bind error");
    
    if (listen(serverSock, 5) < 0) handleError("listen error");

    printf("TCP 서버 대기 중 (Port: %d)...\n", PORT);

    for (;;) {
        cliLen = sizeof(cliAddr);
        clientSock = accept(serverSock, (struct sockaddr*)&cliAddr, &cliLen);
        if (clientSock < 0) continue;
        
        printf("클라이언트 접속\n");
        measure(clientSock, showData);
        printf("접속 종료\n");
        close(clientSock);
    }
    return 0;
}