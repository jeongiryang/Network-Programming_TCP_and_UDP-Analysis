#include <stdio.h>      
#include <stdlib.h>     // 에러 나면 프로그램 종료(exit 함수) 용
#include <string.h>     // 메모리 초기화하거나 글자 복사하는 함수용
#include <unistd.h>     // 소켓 닫는 close 함수 쓰려고 추가함
#include <arpa/inet.h>  // 주소 숫자로 바꾸거나 순서 맞추는 함수들 들어있음
#include <sys/socket.h> // 소켓 만들고 데이터 받는 함수들 모음
#include <time.h>       // 1초마다 전송 속도 계산하려고 시간 체크용도

#define PORT 2022       // 서버 포트 번호 
#define BUFSIZE 2017    // 내 학번 뒷자리로 정한 버퍼 크기. 실습 조건에서 최대 바이트(2000)까지 가능

// 에러 메시지 띄우고 바로 꺼버리는 함수
void handleError(const char *msg) {
    perror(msg);
    exit(1);
}

// UDP 데이터 받아서 에코에서 다시 전송하고 속도 측정하는 함수
void measure(int serverSock, int showData) {
    char buffer[BUFSIZE];           // 데이터 임시로 담는 곳
    struct sockaddr_in cliAddr;     // 데이터 보낸 사람 정보 담아두는 상자
    socklen_t cliLen;               
    int totalBytes = 0;             // 1초 동안 받은 데이터양 합계
    time_t startTime = time(NULL);  // 측정 시작 시간 기록
    ssize_t readLen;                
    char sampleData[101] = {0};     

    // UDP 서버는 미리 연결 안 하고 그냥 무한 루프로 패킷 들어오는 거 기다림
    for (;;) {
        cliLen = sizeof(cliAddr);
        // recvfrom: 누가 보냈는지 주소 정보랑 데이터를 한 번에 받는 함수
        readLen = recvfrom(serverSock, buffer, BUFSIZE - 1, 0, (struct sockaddr*)&cliAddr, &cliLen);
        
        if (readLen > 0) {
            // 에코 서버니까 받은 데이터를 그대로 클라이언트한테 다시 쏴줌
            // 방금 recvfrom으로 알아낸 클라이언트 주소(cliAddr)를 목적지로 씀
            sendto(serverSock, buffer, readLen, 0, (struct sockaddr*)&cliAddr, cliLen);

            buffer[readLen] = '\0'; 
            totalBytes += readLen;  // 속도 계산하려고 받은 양을 계속 더함
            
            if (showData) strncpy(sampleData, buffer, 100);

            time_t currentTime = time(NULL);
            // 딱 1초 지날 때마다 지금까지 모인 데이터 양을 화면에 출력함
            if (currentTime - startTime >= 1) {
                if (showData) {
                    printf("수신 데이터 샘플: %s\n", sampleData);
                }
                // 실험 결과 UDP는 손실률 1% 환경에서도 TCP보다 속도가 훨씬 잘 나오는 게 확인됨
                printf("UDP Throughput: %d bytes/sec\n", totalBytes);
                printf("------------------------------------\n");
                
                totalBytes = 0;          // 다음 1초 측정하려고 다시 0으로 비움
                startTime = currentTime; // 시간 기준점도 지금으로 바꿈
            }
        }
    }
}

int main() {
    int serverSock, showData;       
    struct sockaddr_in servAddr;    // 서버 주소 정보 담는 상자 같은 거

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
    // 컴퓨터마다 숫자 읽는 순서 다를 수 있어서 하나로 맞추는 작업
    servAddr.sin_port = htons(PORT);

    // 내가 정한 주소랑 포트를 소켓에 딱 붙여줌
    if (bind(serverSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
        handleError("bind error");

    printf("UDP 서버 대기 중 (Port: %d)...\n", PORT);
    // UDP는 기다리는 함수(listen) 없이 바로 데이터 받기 시작하면 됨
    measure(serverSock, showData);

    // 다 썼으면 소켓 반납
    close(serverSock);
    return 0;
}