# TCP / UDP Performance Analysis

C 소켓 프로그램으로 TCP·UDP의 에코 모드와 단방향 전송을 비교한 네트워크 실험입니다. 송신 속도와 NetEM 지연·손실 조건을 바꾸며 실시간·누적 평균 처리량을 기록했습니다.

![C](https://img.shields.io/badge/C-A8B9CC?style=flat-square&logo=c&logoColor=black)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=flat-square&logo=linux&logoColor=black)
![NetEM](https://img.shields.io/badge/NetEM-555555?style=flat-square)

[컴파일·실행](#실행) · [TCP 소스](tcp) · [UDP 소스](udp) · [전체 분석 보고서](REPORT.md)

| TCP · 손실 조건 비교 | UDP · 손실 조건 비교 |
| --- | --- |
| ![TCP 처리량 실험 그래프](screenshots/graph/graph_2_tcp_1.0.png) | ![UDP 처리량 실험 그래프](screenshots/graph/graph_4_udp_1.0.png) |

## 실험 구성

| 항목 | 설정 |
| --- | --- |
| 통신 | `127.0.0.1:2022` / TCP·UDP |
| 모드 | `1` 에코 / `2` 단방향 수신 |
| 목표 송신량 | 초당 `500` / `1000` / `2000` bytes |
| 네트워크 조건 | NetEM 지연 설정 `100ms`, 손실률 `0.1%` / `1%` |
| 기록 | 1초 단위 처리량, 시작 이후 누적 평균 |

보고서에는 각 조건의 측정 화면과 4개 비교 그래프가 있습니다. 결과는 이 프로그램의 전송 주기와 실험 환경을 기준으로 읽습니다.

## 실행

기존 실험 환경은 **WSL2 / Ubuntu 24.04 / GCC 13.3.0**입니다. 저장소 루트에서 컴파일합니다.

```bash
gcc tcp/tcp_server.c -o tcp_server
gcc tcp/tcp_client.c -o tcp_client
gcc udp/udp_server.c -o udp_server
gcc udp/udp_client.c -o udp_client
```

서로 다른 터미널에서 서버를 먼저 실행하고 클라이언트를 연결합니다. 양쪽에서 같은 모드와 데이터 출력 여부를 선택합니다.

```bash
# 터미널 1
./tcp_server

# 터미널 2
./tcp_client 500
```

UDP는 `./udp_server`, `./udp_client 500`으로 실행합니다. NetEM 설정·해제 명령은 아래 환경 기록에 있습니다.

## 결과와 자료

- [처리량 측정 결과](REPORT.md#3-전송-속도-및-패킷-손실률-변화에-따른-throughput-측정-결과)
- [모드별 결과 분석](REPORT.md#4-결과-분석-tcp와-udp의-차이-및-모드별-특성)
- [측정 화면과 그래프](screenshots)

## 과제 보고서

<details>
<summary>기존 과제 보고서 펼치기</summary>

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
  학번 뒷자리를 반영한 **BUFSIZE(2017)** 를 사용하며, 데이터를 수신한 후 즉시 삭제 처리한다.  
  1초 단위로 실시간 **Throughput(bytes/sec)** 을 계산하여 출력한다.

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


### 장애 설정 추가 (RTT 100ms, 패킷 손실 0.1% 또는 1%)
```bash
sudo tc qdisc add dev lo root netem delay 100ms loss 1% # 1%
```

### 장애 설정 변경 (RTT 100ms, 패킷 손실 0.1% 또는 1%)
```bash
sudo tc qdisc change dev lo root netem delay 100ms loss 0.1% # 1%
```

### 장애 설정 적용 후 확인 명령어
```bash
tc qdisc show dev lo
```

### 장애 설정 삭제 (실험 종료 후 초기화 필수)
```bash
sudo tc qdisc del dev lo root
```

</details>
