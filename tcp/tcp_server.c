#include <stdio.h>      
#include <stdlib.h>     // 에러 났을 때 프로그램 끄려고 추가함
#include <string.h>     // 메모리 지우거나 글자 복사하는 함수들 쓰려고 넣음
#include <unistd.h>     // 소켓 닫을 때(close) 필요한 헤더
#include <arpa/inet.h>  // 주소 변환하는 함수랑 상자 같은 구조체 쓰려고 추가
#include <sys/socket.h> // 소켓 통신할 때 필수인 헤더
#include <time.h>       // 1초 지났나 확인해서 속도 계산하려고 넣음
#include <sys/time.h>   // 전체 실행 시간 정밀하게 재서 평균 구하려고 추가함

#define PORT 2022       // 학번 앞자리로 정한 포트 번호
#define BUFSIZE 2017    // 학번 뒷자리로 정한 버퍼 크기. 2000바이트까지 받을 수 있음

// 에러 메시지 띄우고 바로 종료시키는 함수
void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

// 데이터 받고 전송률 계산하는 함수 
void measure(int client, int showData, int mode) {
    char buffer[BUFSIZE];           // 받은 데이터 임시로 담는 곳
    int totalBytes = 0;             // 1초 동안 총 몇 바이트 받았나 기록용
    long long cumulativeTotal = 0;  // 프로그램 시작부터 지금까지 받은 전체 데이터 양
    time_t startTime = time(NULL);  // 1초 수신 측정용 기준 시간
    struct timeval start_t, end_t;  // 전체 평균 계산용 시작/끝 시각 상자
    ssize_t readLen;                
    char sampleData[101] = {0};     

    gettimeofday(&start_t, NULL);   // 클라이언트 접속하자마자 최초 시작 시각 기록

    // 클라이언트랑 연결 끊기 전까지 계속 받음. 받기만 하고 저장은 안 하니까 자동 삭제됨
    while ((readLen = recv(client, buffer, BUFSIZE - 1, 0)) > 0) {
        
        // 모드가 1(에코 모드)일 때만 클라이언트가 보낸 걸 그대로 다시 쏴줌
        if (mode == 1) {
            send(client, buffer, readLen, 0);
        }

        buffer[readLen] = '\0';     
        totalBytes += readLen;      // 1초간 받은 양 누적
        cumulativeTotal += readLen; // 전체 받은 양 누적 (평균 계산용)
        
        if (showData) {
            strncpy(sampleData, buffer, 100); // 출력 모드면 앞부분만 살짝 복사
        }

        time_t currentTime = time(NULL); 
        // 딱 1초 지날 때마다 지금까지 쌓인 데이터 양이랑 '누적 평균'을 한 줄에 찍음
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

int main() {
    int serverSock, clientSock, showData, mode; // 실행 모드 저장할 mode 변수 추가
    struct sockaddr_in servAddr, cliAddr; // 주소 정보 저장하는 상자 같은 거
    socklen_t cliLen;

    // 에코할지 그냥 받을지만 할지 물어봄
    printf("모드 선택 (1: 에코 모드, 2: 단방향 모드): ");
    if (scanf("%d", &mode) != 1) mode = 1;

    printf("데이터 출력? (1: 예, 0: 아니오): ");
    if (scanf("%d", &showData) != 1) showData = 0;

    // IPv4랑 TCP 방식으로 소켓 만듦
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) handleError("socket error");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    // 어떤 IP로 들어오든 다 받겠다는 설정
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 컴퓨터마다 숫자 읽는 순서가 다를 수 있어서 하나로 맞춤
    servAddr.sin_port = htons(PORT);

    // 내가 정한 주소랑 포트에 소켓을 연결함
    if (bind(serverSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) 
        handleError("bind error");
    
    // 클라이언트 기다리는 대기 상태로 바꿈. 최대 5명까지 줄 세울 수 있음
    if (listen(serverSock, 5) < 0) handleError("listen error");

    // 선택한 모드에 맞춰서 대기 메시지 다르게 띄움
    if (mode == 1) printf("TCP 서버 대기 중 (에코 모드, Port: %d)...\n", PORT);
    else printf("TCP 서버 대기 중 (단방향 측정 모드, Port: %d)...\n", PORT);

    for (;;) {
        cliLen = sizeof(cliAddr);
        // 실제로 접속 요청이 오면 통신용 소켓을 새로 하나 만들어줌
        clientSock = accept(serverSock, (struct sockaddr*)&cliAddr, &cliLen);
        if (clientSock < 0) continue;
        
        printf("클라이언트 접속\n");
        // 여기서 실제로 데이터 받고 모드에 따라 다시 주거나(에코) 속도 측정함
        measure(clientSock, showData, mode);
        
        // 통신 끝나면 소켓 닫고 다시 루프 돌면서 다음 접속 기다림
        printf("접속 종료\n");
        close(clientSock);
    }
    return 0;
}