#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IP "127.0.0.1"
#define PORT 2022

void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

void sendData(int sock, int speed) {
    char *buffer = (char*)malloc(speed);
    if (buffer == NULL) handleError("malloc error");
    memset(buffer, 'X', speed);

    printf("목표 전송 속도: %d bytes/sec\n", speed);

    for (;;) {
        ssize_t sentLen = send(sock, buffer, speed, 0);
        if (sentLen <= 0) break;
        sleep(1); 
    }
    free(buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <speed>\n", argv[0]);
        return 1;
    }

    int sendSpeed = atoi(argv[1]);
    int clientSock;
    struct sockaddr_in servAddr;

    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock < 0) handleError("socket error");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(IP);
    servAddr.sin_port = htons(PORT);

    if (connect(clientSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) != 0)
        handleError("connect error");
    
    printf("서버 연결 성공\n");
    sendData(clientSock, sendSpeed);

    close(clientSock);
    return 0;
}