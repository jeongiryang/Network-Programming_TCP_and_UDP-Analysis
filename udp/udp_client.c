#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IP "127.0.0.1"
#define PORT 2022
#define SPLIT 10

// 에러 처리용 매크로
#define CHECK(cond, msg) if(cond) { perror(msg); exit(1); }

// 지정된 속도로 UDP 데이터를 분할 전송하는 함수
void senddata(int client, struct sockaddr_in *dest, int speed) {
    int chunksize = speed / SPLIT;
    int delayus = 100000; // 0.1초 대기

    // 메모리 동적 할당 및 더미 데이터 생성
    char *buffer = (char*)malloc(chunksize);
    memset(buffer, 'U', chunksize);

    printf("목표 속도: %d B/s (1초에 %d B씩 %d번 전송)\n", speed, chunksize, SPLIT);

    for (;;) {
        // UDP는 sendto를 사용하여 목적지 주소로 바로 전송
        ssize_t sent = sendto(client, buffer, chunksize, 0, (struct sockaddr*)dest, sizeof(*dest));
        if (sent <= 0) {
            printf("데이터 전송 에러\n");
            break;
        }
        usleep(delayus);
    }

    free(buffer);
}

int main(int argc, char *argv[]) {
    // 인자값 확인
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

    printf("UDP 전송 준비 완료\n");

    // 데이터 전송 로직 호출
    senddata(client, &servaddr, speed);

    close(client);
    return 0;
}