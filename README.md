# Network Programming HW01: TCP vs UDP Performance Analysis

## 환경 구축
- **OS**: Windows 11 (WSL2 - Ubuntu 24.04)
- **Compiler**: GCC 13.3.0
- **Network Tool**: Linux tc (NetEM)

## 폴더 구조 및 파일 설명

### 1. tcp_part/ (TCP 통신 구현부)
- **tcp_server.c**: TCP 에코 서버다. 클라이언트의 접속을 수락하고 수신된 데이터를 읽어들임. 1초 단위로 누적 수신량(Throughput)을 계산하여 터미널에 출력.
- **tcp_client.c**: TCP 클라이언트다. 실행 시 인자로 목표 속도(500, 1000, 2000 B/s)를 입력받는다. 네트워크 부하를 부드럽게 분산하기 위해 목표 속도의 데이터를 1초에 10번(`usleep(100000)`)으로 나누어 전송하는 속도 제어 로직을 적용.

### 2. udp_part/ (UDP 통신 구현부)
- **udp_server.c**: UDP 서버다. `accept` 과정 없이 `recvfrom`을 통해 데이터를 즉시 수신하며, TCP 서버와 동일하게 1초 단위로 Throughput을 측정해 출력.
- **udp_client.c**: UDP 클라이언트다. TCP 클라이언트와 동일한 조건(1초에 10번 분할 전송)으로 속도 제어 로직을 적용했으며, `sendto`를 사용해 서버로 데이터를 전송.

## 컴파일 및 실행 방법

### TCP 테스트
```bash
# 컴파일
gcc -o tcp_server tcp_server.c
gcc -o tcp_client tcp_client.c

# 실행 (서버를 먼저 실행하여 대기)
./tcp_server

# 실행 (클라이언트 - 500, 1000, 2000 중 속도 지정)
./tcp_client 500
```

### UDP 테스트
```bash
# 컴파일
gcc -o udp_server udp_server.c
gcc -o udp_client udp_client.c

# 실행 (서버를 먼저 실행하여 대기)
./udp_server

# 실행 (클라이언트 - 500, 1000, 2000 중 속도 지정)
./udp_client 500
```

## NetEM 네트워크 제어 명령어

과제의 핵심인 혼잡 제어(Congestion Control) 실험을 위해 로컬 루프백(`lo`) 인터페이스에 인위적인 네트워크 장애를 가함. 터미널을 분할하여 아래 명령어들을 사용.

### 장애 설정 추가 (RTT 100ms, 패킷 손실 1%)
```bash
sudo tc qdisc add dev lo root netem delay 100ms loss 1%
```

### 장애 설정 변경 (패킷 손실 0.1%로 변경)
```bash
sudo tc qdisc change dev lo root netem delay 100ms loss 0.1%
```

### 장애 설정 삭제 (실험 종료 후 초기화 필수)
```bash
sudo tc qdisc del dev lo root
```