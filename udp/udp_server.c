#include <stdio.h>      
#include <stdlib.h>     // 에러 나면 프로그램 종료(exit 함수) 용
#include <string.h>     // 메모리 초기화하거나 글자 복사하는 함수용
#include <unistd.h>     // 소켓 닫는 close 함수 쓰려고 추가함
#include <arpa/inet.h>  // 주소 숫자로 바꾸거나 순서 맞추는 함수들 들어있음
#include <sys/socket.h> // 소켓 만들고 데이터 받는 함수들 모음
#include <time.h>       // 1초마다 전송 속도 계산하려고 시간 체크용도
#include <sys/time.h>   // 전체 시간 정밀하게 재서 평균 구하려고 추가함

#define PORT 2022       // 서버 포트 번호 
#define BUFSIZE 2017    // 내 학번 뒷자리로 정한 버퍼 크기. 실습 조건에서 최대 바이트(2000)까지 가능

// 에러 메시지 띄우고 바로 꺼버리는 함수
void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

void measure(int serverSock, int showData, int mode) {
    char buffer[BUFSIZE];           // 데이터 임시로 담는 곳
    struct sockaddr_in cliAddr;     // 데이터 보낸 사람 정보 담아두는 상자
    socklen_t cliLen;               
    int totalBytes = 0;             // 1초 동안 받은 데이터양 합계
    long long cumulativeTotal = 0;  // 프로그램 시작부터 지금까지 받은 전체 데이터 양
    time_t startTime = time(NULL);  // 1초 수신 측정용 기준 시간
    struct timeval start_t, end_t;  // 전체 평균 계산용 시작/끝 시각 상자
    ssize_t readLen;                
    char sampleData[101] = {0};     

    gettimeofday(&start_t, NULL);   // 첫 패킷 받기 전 일단 기준 시각 기록함

    // UDP 서버는 미리 연결 안 하고 그냥 무한 루프로 패킷 들어오는 거 기다림
    for (;;) {
        cliLen = sizeof(cliAddr);
        // recvfrom: 누가 보냈는지 주소 정보랑 데이터를 한 번에 받는 함수
        readLen = recvfrom(serverSock, buffer, BUFSIZE - 1, 0, (struct sockaddr*)&cliAddr, &cliLen);
        
        if (readLen > 0) {
            // 모드가 1(에코 모드)일 때만 받은 데이터를 그대로 클라이언트한테 다시 쏴줌
            if (mode == 1) {
                // 방금 recvfrom으로 알아낸 클라이언트 주소(cliAddr)를 목적지로 씀
                sendto(serverSock, buffer, readLen, 0, (struct sockaddr*)&cliAddr, cliLen);
            }

            buffer[readLen] = '\0'; 
            totalBytes += readLen;      // 1초간 받은 양 누적
            cumulativeTotal += readLen; // 전체 받은 양 누적 (평균 계산용)
            
            if (showData) strncpy(sampleData, buffer, 100);

            time_t currentTime = time(NULL);
            // 딱 1초 지날 때마다 지금까지 쌓인 데이터 양이랑 '누적 평균'을 한 줄에 출력함
            if (currentTime - startTime >= 1) {
                gettimeofday(&end_t, NULL); // 현재 시각 딱 재서
                // 시작부터 지금까지 흐른 시간(초 단위) 계산함
                double elapsed = (end_t.tv_sec - start_t.tv_sec) + (end_t.tv_usec - start_t.tv_usec) / 1000000.0;
                // 전체 받은 양을 흐른 시간으로 나눠서 진짜 평균 수신 속도 구함
                double avgThroughput = cumulativeTotal / elapsed;

                if (showData) {
                    printf("수신 데이터 샘플: %s\n", sampleData);
                }
                
                // 한 줄에 [시간, 실시간, 누적 평균] 싹 다 보여줌
                printf("[시간: %5.1f초] 실시간: %5d B/s | 누적 평균: %8.2f B/s | ", elapsed, totalBytes, avgThroughput);
                
                // 500바이트당 네모 1개씩 출력하여 시각화
                for (int i = 0; i < totalBytes / 500; i++) {
                    printf("■");
                }
                printf("\n");
                
                printf("------------------------------------\n");
                
                totalBytes = 0;          // 다음 1초 측정하려고 다시 0으로 비움
                startTime = currentTime; // 시간 기준점도 지금으로 바꿈
            }
        }
    }
}

int main() {
    int serverSock, showData, mode; // 실행 모드 저장할 mode 변수 추가
    struct sockaddr_in servAddr;    // 서버 주소 정보 담는 상자 같은 거임

    // 에코할지 그냥 받을지만 할지 모드 먼저 물어봄
    printf("모드 선택 (1: 에코 모드, 2: 단방향 모드): ");
    if (scanf("%d", &mode) != 1) mode = 1;

    printf("데이터 출력? (1: 예, 0: 아니오): ");
    if (scanf("%d", &showData) != 1) showData = 0;

    // IPv4 주소랑 UDP 방식(DGRAM) 소켓 만들기
    serverSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSock < 0) handleError("socket error");

    // 주소 상자 초기화하고 내 정보 채워넣음
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    // 어떤 IP로 들어오는 접속이든 다 받겠다는 설정
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 컴퓨터마다 숫자 읽는 순서가 다를 수 있어서 하나로 맞추는 작업
    servAddr.sin_port = htons(PORT);

    // 내가 정한 주소랑 포트를 소켓에 딱 붙여줌
    if (bind(serverSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
        handleError("bind error");

    // 선택한 모드에 맞춰서 대기 메시지 다르게 띄움
    if (mode == 1) printf("UDP 서버 대기 중 (에코 모드, Port: %d)...\n", PORT);
    else printf("UDP 서버 대기 중 (단방향 측정 모드, Port: %d)...\n", PORT);
    
    // 유디피는 기다리는 함수(listen) 없이 바로 데이터 받기 시작하면 됨
    measure(serverSock, showData, mode);

    // 다 썼으면 소켓 반납
    close(serverSock);
    return 0;
}