#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 2022
#define BUFSIZE 4096

// 이량 님의 원본 코드 로직 완벽 적용
void measure(int client) {
    char buffer[BUFSIZE];
    int total = 0;
    time_t start = time(NULL); // 루프 진입 전 타이머 시작 (원본과 동일)
    ssize_t len;

    do {
        len = recv(client, buffer, BUFSIZE, 0);
        if (len > 0) {
            total += len; // 받은 만큼 더하기
            
            time_t now = time(NULL);
            if (now - start >= 1) { // 딱 1초 지나면 출력
                printf("TCP Throughput: %d bytes/sec\n", total);
                total = 0; // 0으로 초기화
                start = now; // 타이머 초기화
            }
        }
    } while (len > 0);
}

int main() {
    int server, client;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) { perror("소켓 에러"); exit(1); }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("바인딩 에러"); exit(1);
    }

    if (listen(server, 5) < 0) { perror("리슨 에러"); exit(1); }

    printf("포트 %d에서 대기 중...\n", PORT);

    for (;;) {
        clilen = sizeof(cliaddr);
        client = accept(server, (struct sockaddr*)&cliaddr, &clilen);
        if (client < 0) continue;
        
        printf("\n클라이언트 연결됨.\n");
        measure(client);
        printf("연결 종료됨.\n");
        close(client);
    }

    close(server);
    return 0;
}