#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BIND_PORT 9090
#define MAX_BUF 4096

int main() {
    int udp_sock;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len;
    char pk_buf[MAX_BUF];

    // 1. UDP용 소켓 만들기 (SOCK_DGRAM이 UDP 방식임)
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP 소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 내 포트번호(9090) 설정하기
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(BIND_PORT);

    // 3. 포트 묶어주기 (UDP는 bind만 하면 바로 받을 준비 끝남)
    if (!(bind(udp_sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == 0)) {
        perror("포트 바인딩 실패");
        exit(EXIT_FAILURE);
    }

    printf("UDP %d 포트에서 데이터 날아오기를 기다리는 중...\n", BIND_PORT);

    int total_rx = 0;
    time_t t_start = time(NULL);

    for (;;) {
        cli_len = sizeof(cli_addr);
        
        // 4. UDP는 연결 과정(accept) 없이 날아오는 패킷을 recvfrom으로 바로 받음
        ssize_t rx_len = recvfrom(udp_sock, pk_buf, MAX_BUF, 0, 
                                  (struct sockaddr*)&cli_addr, &cli_len);
        
        if (rx_len > 0) {
            total_rx += rx_len; // 받은 데이터 양 더하기
            
            time_t t_now = time(NULL);
            
            // 1초 단위로 끊어서 화면에 속도 출력
            if (t_now - t_start >= 1) {
                printf("1초 동안 받은 데이터(UDP): %d bytes/sec\n", total_rx);
                total_rx = 0; // 누적값 0으로 초기화
                t_start = t_now;
            }
        }
    }

    close(udp_sock);
    return 0;
}