#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>       // 1초마다 송신량 계산하려고 추가함
#include <sys/time.h>   // 전체 실행 시간 정밀하게 재서 평균 구하려고 추가함

#include <arpa/inet.h> // 주소 변환해주는 inet_addr 같은 함수 쓰려고 추가함
#include <sys/socket.h> // 소켓 만들고 연결할 때 필요한 함수들이 모여있는 거

#define IP "127.0.0.1" // 내 컴퓨터에서 NetEM으로 장애 환경 테스트하기 위한 주소 
#define PORT 2022 

void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

void sendData(int sock, int speed, int showData, int mode) {
    // 실습 조건: 500, 1000, 2000바이트씩 공간 만들어서 보냄
    char *buffer = (char*)malloc(speed);
    // 서버가 다시 돌려주는 거 받을 공간도 똑같이 만듦 (에코 서버)
    char *recvBuffer = (char*)malloc(speed); 
    char sampleData[101] = {0}; // 출력용 샘플 담을 공간
    
    int totalSentBytes = 0;             // 1초 동안 보낸 데이터양 합계
    long long cumulativeBytes = 0;      // 프로그램 시작부터 지금까지 보낸 전체 데이터 양
    time_t startTime = time(NULL);      // 1초 송신 측정용 기준 시간
    struct timeval start_t, end_t;      // 전체 평균 계산용 시작/끝 시각 상자
    
    gettimeofday(&start_t, NULL);       // 함수 시작하자마자 최초 시작 시각 기록

    if (buffer == NULL || recvBuffer == NULL) handleError("malloc error");
    
    memset(buffer, 'T', speed); // TCP 데이터라서 'T'로 채워넣음

    printf("목표 전송 속도: %d bytes/sec\n", speed);
    if (mode == 2) printf("(단방향 모드 실행 중..)\n");

    for (;;) {
        // 1. 데이터 보내기
        // 실험: TCP는 손실률 1% 환경에서 UDP보다 속도가 확 떨어짐. 다시 보내느라 그런 듯
        ssize_t sentLen = send(sock, buffer, speed, 0);
        if (sentLen <= 0) break;
        
        totalSentBytes += sentLen; // 1초간 보낸 양 누적
        cumulativeBytes += sentLen; // 전체 보낸 양 누적 (평균 계산용)
        
        // 2. 에코 데이터 받기 
        // 모드가 1(에코 모드)일 때만 서버가 돌려준 데이터를 기다렸다가 다시 받음
        if (mode == 1) {
            ssize_t readLen = recv(sock, recvBuffer, speed, 0);
            
            if (readLen > 0) {
                printf("서버로부터 돌려받은 데이터 %ld bytes\n", readLen);
                
                if (showData) {
                    strncpy(sampleData, recvBuffer, 100);
                    printf("에코 데이터 샘플: %s\n", sampleData);
                }
            }
            if (readLen <= 0) break;
        }
        
        time_t currentTime = time(NULL);
        // 1초 지날 때마다 지금까지 쏜 데이터 양이랑 '누적 평균'을 한 줄에 출력함
        if (currentTime - startTime >= 1) {
            gettimeofday(&end_t, NULL); // 현재 시각 딱 재서
            // 시작부터 지금까지 흐른 시간(초 단위) 계산함
            double elapsed = (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_usec - start_t.tv_usec) / 1000000.0;
            // 전체 보낸 양을 흐른 시간으로 나눠서 진짜 평균 속도 구함
            double avgThroughput = cumulativeBytes / elapsed;

            // 한 줄에 [시간, 실시간, 누적 평균] 싹 다 보여줌
            printf("[시간: %5.1f초] 실시간: %5d B/s | 누적 평균: %8.2f B/s | ", elapsed, totalSentBytes, avgThroughput);
            
            // 500바이트당 네모 1개씩 출력하여 시각화
            for (int i = 0; i < totalSentBytes / 500; i++) {
                printf("■");
            }
            printf("\n");

            totalSentBytes = 0;          // 다음 1초 측정하려고 다시 0으로 비움
            startTime = currentTime;     // 1초 기준점도 지금으로 바꿈
        }
        
        // 1초 안 쉬고 그냥 보내면 속도가 너무 제각각이라 1초로 딱 맞춤
        sleep(1); 
    }
    
    free(buffer);
    free(recvBuffer); // 다 썼으면 받은 공간도 비워줌
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <speed>\n", argv[0]);
        return 1;
    }

    int sendSpeed = atoi(argv[1]); 
    int clientSock, showData, mode; 
    struct sockaddr_in servAddr; 

    printf("모드 선택 (1: 에코 모드, 2: 단방향 모드): ");
    if (scanf("%d", &mode) != 1) mode = 1;

    printf("데이터 출력? (1: 예, 0: 아니오): ");
    if (scanf("%d", &showData) != 1) showData = 0;

    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock < 0) handleError("socket error");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(IP);
    servAddr.sin_port = htons(PORT);

    if (connect(clientSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) != 0)
        handleError("connect error");
    
    if (mode == 1) printf("서버 연결 성공 (에코 모드)\n");
    else printf("서버 연결 성공 (단방향 측정 모드)\n");
    
    sendData(clientSock, sendSpeed, showData, mode);

    close(clientSock); 
    return 0;
}