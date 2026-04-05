#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DEST_IP "127.0.0.1"
#define DEST_PORT 8080

int main(int argc, char *argv[]) {
    // 터미널에서 실행할 때 속도 입력 안 하면 사용법 알려주고 종료
    if (argc != 2) {
        fprintf(stderr, "실행 방법: %s <보낼 속도(500/1000/2000)>\n", argv[0]);
        return 1;
    }

    // 입력받은 속도 숫자로 변환
    int target_speed = atoi(argv[1]);
    int cli_sock;
    struct sockaddr_in srv_info;

    // 1. 서버에 접속할 소켓 만들기
    if ((cli_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("소켓 생성 실패");
        return 1;
    }

    // 2. 찾아갈 서버의 IP 주소와 포트번호 설정
    memset(&srv_info, 0, sizeof(srv_info));
    srv_info.sin_family = AF_INET;
    srv_info.sin_addr.s_addr = inet_addr(DEST_IP);
    srv_info.sin_port = htons(DEST_PORT);

    // 3. 서버에 연결 요청하기 
    if (connect(cli_sock, (struct sockaddr*)&srv_info, sizeof(srv_info)) != 0) {
        perror("서버 연결 실패");
        return 1;
    }
    
    printf("서버 연결 성공!\n");

    // ===== 과제 요구사항: 지정된 속도로 데이터 보내기 =====
    #define SPLIT_FACTOR 10 
    int snd_chunk = target_speed / SPLIT_FACTOR; // 한 번에 보낼 크기 (예: 50바이트)
    
    // 1초(100만 마이크로초)를 10등분한 시간 -> 0.1초(10만 마이크로초) 대기
    int delay_tick = 100000; 

    // 보낼 가짜 데이터 버퍼 만들고 'X'로 꽉 채우기
    char *payload = (char*)malloc(snd_chunk);
    memset(payload, 'X', snd_chunk); 

    printf("목표 속도: %d B/s (한 번에 %d B씩 10번 나눠서 보냄)\n", target_speed, snd_chunk);

    // 계속해서 서버로 데이터 쏘기
    for (;;) {
        ssize_t snt_len = send(cli_sock, payload, snd_chunk, 0);
        if (snt_len <= 0) {
            printf("서버랑 연결 끊김. 전송 멈춤.\n");
            break;
        }
        
        // 0.1초씩 쉬면서 전송 속도를 맞춤
        usleep(delay_tick); 
    }

    free(payload);
    close(cli_sock);
    return 0;
}