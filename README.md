# Network Programming HW01: TCP vs UDP Performance Analysis

## 환경 구축
- **언어**: C 프로그래밍 언어
- **OS**: Windows 11 (WSL2 - Ubuntu 24.04)
- **Compiler**: GCC 13.3.0
- **Network Tool**: Linux tc (NetEM)

## 폴더 구조 및 파일 설명

### 1. tcp/ (TCP 통신 구현부)
- **tcp_server.c**: 한 번에 하나의 클라이언트와 통신하는 TCP 에코 서버다.  
  실행 시 데이터 출력 여부를 선택할 수 있다.  
  학번 뒷자리를 반영한 **BUFSIZE(2017)**를 사용하며, 데이터를 수신한 후 즉시 삭제 처리한다.  
  1초 단위로 실시간 Throughput(bytes/sec)을 계산하여 출력한다.

- **tcp_client.c**: 지정된 속도로 데이터를 전송하는 TCP 클라이언트다.  
  목표 속도(500, 1000, 2000 B/s)의 데이터를 1초마다 통째로 전송한 뒤 `sleep(1)`을 호출하여 정밀한 속도 제어를 수행한다.

### 2. udp/ (UDP 통신 구현부)
- **udp_server.c**: 한 번에 하나의 클라이언트와 통신하는 UDP 에코 서버다.  
  TCP 서버와 동일하게 데이터 수신 후 삭제 및 1초 단위 Throughput 측정 기능을 제공한다.

- **udp_client.c**: 지정된 속도로 데이터를 전송하는 UDP 클라이언트다.  
  `sendto`를 사용하며, TCP 클라이언트와 동일하게 `sleep(1)` 기반의 정밀 전송 로직을 적용한다.

## 컴파일 및 실행 방법

### TCP 테스트

```bash
# 컴파일
gcc -o tcp_server tcp_server.c
gcc -o tcp_client tcp_client.c

# 서버 실행 (데이터 출력 유무 선택: 1 또는 0)
./tcp_server

# 클라이언트 실행 (500, 1000, 2000 중 속도 입력)
./tcp_client 500
```

### UDP 테스트

```bash
# 컴파일
gcc -o udp_server udp_server.c
gcc -o udp_client udp_client.c

# 서버 실행
./udp_server

# 클라이언트 실행 (500, 1000, 2000 중 속도 입력)
./udp_client 500 
```

## NetEM 네트워크 제어 명령어

실험 조건인 RTT 100ms 및 패킷 손실률(0.1%, 1%) 환경을 구축하기 위해 사용한다.

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
