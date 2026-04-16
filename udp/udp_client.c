#include <stdio.h>      
#include <stdlib.h>     // 글자를 숫자로 바꾸는 atoi 같은 거 쓰려고 추가
#include <string.h>     // 메모리 비우거나 글자 복사할 때 필요함
#include <unistd.h>     // sleep으로 시간 멈추거나 소켓 닫을 때 사용
#include <time.h>       // 1초마다 송신량 계산하려고 추가함
#include <sys/time.h>   // 전체 실행 시간 정밀하게 재서 평균 구하려고 추가함
#include <arpa/inet.h>  // 주소 글자를 숫자로 바꾸거나 순서 맞출 때 필요함
#include <sys/socket.h> // 소켓 만들고 데이터 보낼 때 쓰는 함수들 모음

#define IP "127.0.0.1" // 내 컴퓨터 안에서 테스트하기 위한 주소
#define PORT 2022 // 데이터를 보낼 서버 포트 번호

// 에러 나면 메시지 띄우고 바로 꺼버리는 함수
void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

// UDP 전송 함수
void sendData(int clientSock, struct sockaddr_in *destAddr, int speed, int showData, int mode) {
    // 실습 조건인 500, 1000, 2000바이트만큼 데이터 보낼 공간 할당
    char *buffer = (char*)malloc(speed);
    // 에코 서버가 돌려주는 것을 받을 공간도 똑같은 크기로 같이 만듦
    char *recvBuffer = (char*)malloc(speed);
    char sampleData[101] = {0};
    
    int totalSentBytes = 0;             // 1초 동안 보낸 데이터양 합계
    long long cumulativeBytes = 0;      // 프로그램 시작부터 지금까지 쏜 전체 데이터 양
    time_t startTime = time(NULL);      // 1초 송신 측정용 기준 시간
    struct timeval start_t, end_t;      // 전체 평균 계산용 시작/끝 시각 상자
    
    gettimeofday(&start_t, NULL);       // 함수 시작하자마자 최초 시작 시각 기록
    
    if (buffer == NULL || recvBuffer == NULL) handleError("malloc error");
    
    // UDP 데이터라서 'U'로 채워넣음
    memset(buffer, 'U', speed);

    printf("목표 전송 속도: %d bytes/sec\n", speed);
    if (mode == 2) printf("(단방향 모드 실행 중...)\n");

    for (;;) {
        // UDP는 미리 연결을 안 하니까 보낼 때마다 어디로 보낼지 상자(destAddr)를 계속 알려줘야 함
        ssize_t sentLen = sendto(clientSock, buffer, speed, 0, (struct sockaddr*)destAddr, sizeof(*destAddr));
        
        // 전송 실패하면 반복문 탈출
        if (sentLen <= 0) break;

        totalSentBytes += sentLen;   // 1초간 보낸 양 누적
        cumulativeBytes += sentLen;  // 전체 보낸 양 누적 (평균 계산용)

        // 모드가 1(에코 모드)일 때만 방금 보낸 데이터를 서버한테서 바로 다시 받아야 함
        if (mode == 1) {
            // UDP는 받을 때도 보낸 사람 주소를 담을 곳이 필요해서 빈 상자를 하나 만들어서 넣어줌
            struct sockaddr_in fromAddr;
            socklen_t fromLen = sizeof(fromAddr);
            ssize_t readLen = recvfrom(clientSock, recvBuffer, speed, 0, (struct sockaddr*)&fromAddr, &fromLen);
            
            if (readLen > 0) {
                printf("서버로부터 돌려받은 데이터 %ld bytes\n", readLen);
                
                if (showData) {
                    strncpy(sampleData, recvBuffer, 100);
                    printf("에코 데이터 샘플: %s\n", sampleData);
                }
            }
            // 에코 모드인데 받는 것도 실패하면 무한루프 빠져나감
            if (readLen <= 0) break;
        }
        
        time_t currentTime = time(NULL);
        // 1초 지날 때마다 지금까지 쏜 데이터 양이랑 '누적 평균'을 한 줄에 출력함
        if (currentTime - startTime >= 1) {
            gettimeofday(&end_t, NULL); // 현재 시각 딱 재서
            // 시작부터 지금까지 흐른 시간(초 단위) 계산함
            double elapsed = (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_usec - start_t.tv_usec) / 1000000.0;
            // 전체 보낸 양을 흐른 시간으로 나눠서 진짜 평균 전송 속도 구함
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
        
        // 1초 안 쉬고 그냥 보내면 속도가 제각각이라 1초마다 보내게 딱 맞춤
        sleep(1);
    }
    
    // 다 썼으면 할당했던 메모 공간 싹 비워줌
    free(buffer);
    free(recvBuffer);
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

    clientSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSock < 0) handleError("socket error");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(IP);
    servAddr.sin_port = htons(PORT);

    if (mode == 1) printf("UDP 전송 준비 완료 (에코 모드)\n");
    else printf("UDP 전송 준비 완료 (단방향 모드)\n");
    
    sendData(clientSock, &servAddr, sendSpeed, showData, mode);

    close(clientSock);
    return 0;
}