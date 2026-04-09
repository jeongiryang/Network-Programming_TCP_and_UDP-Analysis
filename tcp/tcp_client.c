#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IP "127.0.0.1"
#define PORT 2022
#define CHECK(cond, msg) if(cond) { perror(msg); exit(1); }

void senddata(int sock, int speed) {
    char *buffer = (char*)malloc(speed);
    if (buffer == NULL) exit(1);
    memset(buffer, 'X', speed);

    printf("목표 속도: %d B/s (1초마다 전송)\n", speed);

    for (;;) {
        ssize_t sent = send(sock, buffer, speed, 0);
        if (sent <= 0) {
            printf("연결 종료\n");
            break;
        }
        sleep(1); 
    }
    free(buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "사용법: %s <속도>\n", argv[0]);
        return 1;
    }

    int speed = atoi(argv[1]);
    int client;
    struct sockaddr_in servaddr;

    client = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(client < 0, "소켓 에러");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);

    CHECK(connect(client, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0, "연결 에러");
    
    printf("서버 접속 완료\n");
    senddata(client, speed);

    close(client);
    return 0;
}