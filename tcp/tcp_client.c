#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// 주소 변환해주는 inet_addr 같은 함수 쓰려고 추가함
#include <arpa/inet.h>
// 소켓 만들고 연결할 때 필요한 함수들이 모여있는 헤더
#include <sys/socket.h>

#define IP "127.0.0.1" // 내 컴퓨터에서 NetEM으로 장애 환경 테스트하려고 루프백 주소 사용
#define PORT 2022 

void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

void sendData(int sock, int speed) {
    // 실습에서 하라는 대로 500, 1000, 2000바이트씩 공간 만들어서 보냄
    char *buffer = (char*)malloc(speed);
    if (buffer == NULL) handleError("malloc error");
    
    memset(buffer, 'T', speed);

    printf("목표 전송 속도: %d bytes/sec\n", speed);

    for (;;) {
        // 실험: TCP는 손실률 1% 환경에서 UDP보다 속도가 확 떨어짐. 다시 보내느라 그런 듯
        ssize_t sentLen = send(sock, buffer, speed, 0);
        
        if (sentLen <= 0) break;
        
        // 1초 안 쉬고 그냥 보내면 속도가 너무 제각각이라 1초로 딱 맞춤
        sleep(1); 
    }
    free(buffer);
}

int main(int argc, char *argv[]) {
    // 실행할 때 속도값(500, 1000, 2000) 안 넣으면 사용법 터미널에 출력
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <speed>\n", argv[0]);
        return 1;
    }

    int sendSpeed = atoi(argv[1]); // 문자열로 들어온 숫자를 진짜 숫자로 바꿔줌
    int clientSock;
    struct sockaddr_in servAddr; // 서버 주소 정보 저장하는 상자 같은거

    // IPv4 주소 체계랑 TCP 방식 소켓 생성
    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock < 0) handleError("socket error");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    // 주소 글자를 컴퓨터가 알아듣는 이진 데이터로 변환
    servAddr.sin_addr.s_addr = inet_addr(IP);
    // 컴퓨터마다 숫자를 읽는 순서가 다를 수 있어서 하나로 통일해주는 작업
    servAddr.sin_port = htons(PORT);

    // TCP는 실제 데이터를 쏘기 전에 서버랑 먼저 연결이 되어야 함
    if (connect(clientSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) != 0)
        handleError("connect error");
    
    printf("서버 연결 성공\n");
    
    sendData(clientSock, sendSpeed);

    close(clientSock); // 다 썼으면 소켓 닫아서 자원 반납
    return 0;
}