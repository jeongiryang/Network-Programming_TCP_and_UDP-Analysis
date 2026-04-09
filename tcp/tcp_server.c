#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define PORT 2022
#define BUFSIZE 4096

void measure(int client, int show_data) {
    char buffer[BUFSIZE];
    int total = 0;
    time_t start = time(NULL);
    ssize_t len;
    char last_data[101] = {0}; // 마지막 데이터를 보관할 샘플 버퍼

    while ((len = recv(client, buffer, BUFSIZE - 1, 0)) > 0) {
        buffer[len] = '\0';
        total += len;
        
        // 출력용 샘플 저장
        if (show_data) {
            strncpy(last_data, buffer, 100); 
        }

        time_t now = time(NULL);
        if (now - start >= 1) {
            // 1초마다 출력할 때만 데이터 내용을 살짝 보여줌
            if (show_data) {
                printf("수신 데이터 샘플: %s\n", last_data);
            }
            printf("TCP Throughput: %d bytes/sec\n", total);
            printf("------------------------------------\n");
            
            total = 0;
            start = now;
        }
    }
}

int main() {
    int server, client, show_data;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;

    printf("데이터 내용을 실시간으로 보시겠습니까? (1: 예, 0: 아니오): ");
    scanf("%d", &show_data);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) exit(1);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) exit(1);
    if (listen(server, 5) < 0) exit(1);

    printf("포트 %d 대기 중...\n", PORT);

    for (;;) {
        clilen = sizeof(cliaddr);
        client = accept(server, (struct sockaddr*)&cliaddr, &clilen);
        if (client < 0) continue;
        
        printf("\n클라이언트 연결 성공\n");
        measure(client, show_data);
        printf("연결 종료\n");
        close(client);
    }
    return 0;
}