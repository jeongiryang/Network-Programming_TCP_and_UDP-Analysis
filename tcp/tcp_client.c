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

// 지정된 속도로 데이터를 분할 전송하는 함수
void senddata(int sock, int speed) {
    int chunksize = speed / SPLIT;
    int delayus = 100000; // 0.1초 대기

    // 메모리 동적 할당 및 더미 데이터 생성
    char *buffer = (char*)malloc(chunksize);
    memset(buffer, 'X', chunksize);

    printf("목표 속도: %d B/s (1초에 %d B씩 %d번 전송)\n", speed, chunksize, SPLIT);

    for (;;) {
        ssize_t sent = send(sock, buffer, chunksize, 0);
        if (sent <= 0) {
            printf("서버 연결 끊어짐\n");
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

    client = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(client < 0, "소켓 에러");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(PORT);

    // 서버 연결
    CHECK(connect(client, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0, "연결 에러");
    
    printf("서버 접속 완료\n");

    // 데이터 전송 시작
    senddata(client, speed);

    close(client);
    return 0;
}