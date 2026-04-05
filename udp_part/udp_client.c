#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TARGET_HOST "127.0.0.1"
#define TARGET_PORT 9090

int main(int argc, char *argv[]) {
    // 전송 속도 입력 확인
    if (argc != 2) {
        fprintf(stderr, "실행 방법: %s <500|1000|2000>\n", argv[0]);
        return 1;
    }

    int req_speed = atoi(argv[1]);
    int udp_cli;
    struct sockaddr_in dest_addr;

    // 1. 데이터를 쏠 UDP 소켓 생성
    if ((udp_cli = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP 소켓 생성 에러");
        return 1;
    }

    // 2. 목적지(서버) 주소 미리 셋팅
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(TARGET_HOST);
    dest_addr.sin_port = htons(TARGET_PORT);

    // ===== UDP 전송 속도 맞추기 =====
    // TCP와 동일한 조건으로 실험하기 위해 10번으로 맞춤
    #define UDP_TICKS 10
    int payload_size = req_speed / UDP_TICKS; // 한 번에 던질 크기
    
    // 0.1초 (10만 마이크로초) 대기
    int wait_us = 100000; 

    // 보낼 데이터 버퍼 만들고 'U'로 채움
    char *datagram = (char*)malloc(payload_size);
    memset(datagram, 'U', payload_size); 

    printf("UDP 전송 시작. 목표 속도: %d B/s (한 번에 %d B씩 던짐)\n", req_speed, payload_size);

    for (;;) {
        // 3. UDP는 connect 안 하고, sendto 함수로 목적지 주소 써서 바로 쏴버림
        ssize_t pushed = sendto(udp_cli, datagram, payload_size, 0, 
                                (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        
        if (pushed <= 0) {
            printf("데이터 전송 실패\n");
            break;
        }
        
        // 정해진 시간만큼 잠깐 대기 (속도 맞추기용)
        usleep(wait_us); 
    }

    free(datagram);
    close(udp_cli);
    return 0;
}