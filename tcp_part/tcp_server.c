#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define SRV_PORT 8080
#define RX_BUF_SIZE 4096

int main() {
    int listen_fd, conn_fd;
    struct sockaddr_in srv_addr, cli_addr;
    socklen_t cli_len;
    char rx_buf[RX_BUF_SIZE];

    // 1. 접속을 기다릴 서버용 소켓 만들기 (실패하면 프로그램 종료)
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 사용할 포트번호(8080)와 IP 설정
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(SRV_PORT);

    // 3. 설정한 주소를 소켓에 묶어주기 (bind)
    if (bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) != 0) {
        perror("포트 바인딩 실패");
        exit(EXIT_FAILURE);
    }

    // 4. 클라이언트 접속 대기 모드로 전환 
    if (listen(listen_fd, 5) != 0) {
        perror("대기 모드 전환 실패");
        exit(EXIT_FAILURE);
    }

    printf("포트 %d에서 클라이언트 기다리는 중...\n", SRV_PORT);

    // 5. 무한 루프 돌면서 클라이언트 접속 계속 받기
    for (;;) {
        cli_len = sizeof(cli_addr);
        
        // 접속 요청이 오면 통신용 새 소켓(conn_fd)을 만들어서 연결 수락
        conn_fd = accept(listen_fd, (struct sockaddr*)&cli_addr, &cli_len);
        if (conn_fd < 0) continue; // 에러 나면 무시하고 다음 사람 대기

        printf("\n=> 클라이언트 접속됨. 속도(Throughput) 측정 시작!\n");

        int current_bytes = 0;
        time_t tick_start = time(NULL);
        ssize_t bytes_read;

        // 6. 연결된 클라이언트가 보내는 데이터 계속 읽기
        do {
            bytes_read = recv(conn_fd, rx_buf, RX_BUF_SIZE, 0);
            if (bytes_read > 0) {
                current_bytes += bytes_read; // 받은 만큼 용량 더하기
                
                // 딱 1초가 지났는지 확인하고, 지났으면 화면에 출력
                time_t now = time(NULL);
                if (now - tick_start >= 1) { 
                    printf("1초 동안 받은 데이터: %d bytes/sec\n", current_bytes);
                    current_bytes = 0; // 다음 1초 측정을 위해 0으로 초기화
                    tick_start = now; 
                }
            }
        } while (bytes_read > 0); // 연결 끊길 때까지 반복

        printf("=> 통신 종료됨. 클라이언트 소켓 닫음.\n");
        close(conn_fd);
    }

    close(listen_fd);
    return 0;
}