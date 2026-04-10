#include <stdio.h>      
#include <stdlib.h>     // 글자를 숫자로 바꾸는 atoi 같은 거 쓰려고 추가
#include <string.h>     // 메모리 비우거나 글자 복사할 때 필요함
#include <unistd.h>     // sleep으로 시간 멈추거나 소켓 닫을 때 사용
#include <time.h>       // 1초마다 송신량 계산하려고 추가함
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
void sendData(int clientSock, struct sockaddr_in *destAddr, int speed, int showData) {
    // 실습 조건인 500, 1000, 2000바이트만큼 데이터 보낼 공간 할당
    char *buffer = (char*)malloc(speed);
    // 에코 서버가 돌려주는 것을 받을 공간도 똑같은 크기로 같이 만듦
    char *recvBuffer = (char*)malloc(speed);
    char sampleData[101] = {0};
    
    int totalSentBytes = 0;             // 1초 동안 보낸 데이터양 합계
    time_t startTime = time(NULL);      // 송신 측정 시작 시간
    
    if (buffer == NULL || recvBuffer == NULL) handleError("malloc error");
    
    // UDP 데이터라서 'U'로 채워넣음
    memset(buffer, 'U', speed);

    printf("목표 전송 속도: %d bytes/sec\n", speed);

    for (;;) {
        // UDP는 미리 연결을 안 하니까 보낼 때마다 어디로 보낼지 상자(destAddr)를 계속 알려줘야 함
        ssize_t sentLen = sendto(clientSock, buffer, speed, 0, (struct sockaddr*)destAddr, sizeof(*destAddr));
        
        // 전송 실패하면 반복문 탈출
        if (sentLen <= 0) break;

        totalSentBytes += sentLen; // 보낸 만큼 송신량 누적

        // 에코 클라이언트니까 방금 보낸 데이터를 서버한테서 바로 다시 받아야 함
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
        
        time_t currentTime = time(NULL);
        // 1초 지날 때마다 지금까지 쏜 데이터 양을 화면에 출력한다
        if (currentTime - startTime >= 1) {
            printf("UDP 송신량(Sending Throughput): %d bytes/sec\n", totalSentBytes);
            totalSentBytes = 0;          // 다음 1초 측정하려고 다시 0으로 비움
            startTime = currentTime;     // 시간 기준점도 지금으로 바꿈
        }
        printf("------------------------------------\n");
        
        // 받는 것도 실패하면 무한루프 빠져나감
        if (readLen <= 0) break;

        // 1초 안 쉬고 그냥 보내면 속도가 제각각이라 1초마다 보내게 딱 맞춤
        sleep(1);
    }
    
    // 다 썼으면 할당했던 메모리 공간 싹 비워줌
    free(buffer);
    free(recvBuffer);
}

int main(int argc, char *argv[]) {
    // 실행할 때 속도값(500, 1000, 2000) 안 적으면 사용법 알려줌
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <speed>\n", argv[0]);
        return 1;
    }

    // 터미널에서 글자로 들어온 속도를 진짜 숫자로 변환
    int sendSpeed = atoi(argv[1]);
    int clientSock, showData;
    struct sockaddr_in servAddr; // 주소 정보 담아두는 상자 같은 거

    printf("데이터 출력? (1: 예, 0: 아니오): ");
    if (scanf("%d", &showData) != 1) showData = 0;

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

    printf("UDP 전송 준비 완료 (에코 모드)\n");
    
    // UDP는 TCP랑 다르게 연결 과정 없이 그냥 바로 데이터 쏨
    sendData(clientSock, &servAddr, sendSpeed, showData);

    // 소켓 자원 반납
    close(clientSock);
    return 0;
}