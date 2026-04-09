#include <stdio.h>      
#include <stdlib.h>     // 글자를 숫자로 바꾸는 atoi 같은 거 쓰려고 추가
#include <string.h>     // 메모리 비우거나 글자 복사할 때 필요함
#include <unistd.h>     // sleep으로 시간 멈추거나 소켓 닫을 때 사용
#include <arpa/inet.h>  // 주소 글자를 숫자로 바꾸거나 순서 맞출 때 필요함
#include <sys/socket.h> // 소켓 만들고 데이터 보낼 때 쓰는 함수들 모음

#define IP "127.0.0.1" // 내 컴퓨터 안에서 테스트하려고 루프백 주소로 설정
#define PORT 2022 // 데이터를 보낼 서버 포트 번호

// 에러 나면 메시지 띄우고 바로 꺼버리는 함수
void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

// UDP 전송 함수
void sendData(int clientSock, struct sockaddr_in *destAddr, int speed) {
    // 실습 조건인 500, 1000, 2000바이트만큼 데이터 보낼 공간 할당
    char *buffer = (char*)malloc(speed);
    if (buffer == NULL) handleError("malloc error");
    
    // UDP 데이터라는 거 티 내려고 'U'로 채워넣음
    memset(buffer, 'U', speed);

    printf("목표 전송 속도: %d bytes/sec\n", speed);

    for (;;) {
        // UDP는 미리 연결을 안 하니까 보낼 때마다 어디로 보낼지 상자(destAddr)를 계속 알려줘야 함
        ssize_t sentLen = sendto(clientSock, buffer, speed, 0, (struct sockaddr*)destAddr, sizeof(*destAddr));
        
        // 전송 실패하면 반복문 탈출
        if (sentLen <= 0) break;

        // 1초마다 보내서 초당 전송량을 딱 맞추려고 넣음
        sleep(1);
    }
    // 다 썼으면 메모리 공간 비워줌
    free(buffer);
}

int main(int argc, char *argv[]) {
    // 실행할 때 속도값(500, 1000, 2000) 안 적으면 사용법 알려줌
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <speed>\n", argv[0]);
        return 1;
    }

    // 커맨드 라인에서 글자로 들어온 속도를 진짜 숫자로 변환
    int sendSpeed = atoi(argv[1]);
    int clientSock;
    struct sockaddr_in servAddr; // 주소 정보 담아두는 상자 같은 거

    // IPv4 주소랑 UDP 방식(DGRAM) 소켓 만들기
    clientSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSock < 0) handleError("socket error");

    // 서버 주소 상자 초기화하고 정보 채워넣음
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    // 주소 글자를 컴퓨터가 알아듣게 숫자로 바꿈
    servAddr.sin_addr.s_addr = inet_addr(IP);
    // 컴퓨터마다 숫자 읽는 순서가 다를 수 있어서 하나로 통일하는 작업
    servAddr.sin_port = htons(PORT);

    printf("UDP 전송 준비 완료\n");
    
    // UDP는 TCP랑 다르게 연결(connect) 과정 없이 그냥 바로 데이터 쏨
    sendData(clientSock, &servAddr, sendSpeed);

    // 소켓 자원 반납
    close(clientSock);
    return 0;
}