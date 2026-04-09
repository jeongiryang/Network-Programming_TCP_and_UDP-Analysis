#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IP "127.0.0.1"
#define PORT 2022
#define CHECK(cond, msg) if(cond) { perror(msg); exit(1); }

void senddata(int client, struct sockaddr_in *dest, int speed) {
    char *buffer = (char*)malloc(speed);
    if (buffer == NULL) exit(1);
    memset(buffer, 'U', speed);

    printf("목표 속도: %d B/s (1초마다 전송)\n", speed);

    for (;;) {
        ssize_t sent = sendto(client, buffer, speed, 0, (struct sockaddr*)dest, sizeof(*dest));
        if (sent <= 0) break;
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

    client = socket(AF_INET, SOCK_DGRAM, 0);
    CHECK(client < 0, "소켓 에러");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);

    printf("UDP 전송 시작\n");
    senddata(client, &servaddr, speed);

    close(client);
    return 0;
}