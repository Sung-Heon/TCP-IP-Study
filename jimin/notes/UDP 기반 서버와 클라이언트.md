# TCP vs UDP

TCP 소켓과 UDP 소켓은 다음과 같은 차이점이 있다. 

1. 데이터 송수신 이전과 이후에 거치는 연결 설정 및 해제 과정이 없다. 
2. 데이터 송수신 과정에서 거치는 신뢰성 보장을 위한 흐름 제어가 없다. 

즉, TCP의 가장 큰 특징인 **흐름 제어(Flow Control)**가 UDP에는 존재하지 않는다. 

이 때문에 UDP는 다음과 같은 특징을 가진다. 

1. TCP보다 빠르다. 
2. TCP와 달리, 데이터 전송 과정의 신뢰성을 보장할 수 없다. 

즉, UDP 소켓은 TCP 소켓처럼 패킷에 번호를 붙여 보내거나 응답 메세지를 주고 받지 않기 때문에 데이터가 중간에 손실될 수 있지만, 흐름 제어 과정이 생략되는 만큼 TCP보다 속도가 빠르다.

따라서 데이터가 일부분이라도 손상되어서는 안 되는 압축 파일을 전송하거나 하는 경우에는 무손실 데이터 전송이 보장되는 TCP를 사용해야 한다. 반면, 멀티 미디어 파일과 같이 데이터의 일부가 손실되더라도 크게 문제되지 않지만, 데이터 전송 속도가 관건인 실시간 스트림 서비스와 같은 경우에는 UDP를 사용하는 것이 바람직하다.   

<br />

# UDP 소켓의 송수신 특성

UDP 소켓은 비연결 지향적이다. 즉, TCP 소켓처럼 일대일로 데이터를 전송하는 게 아니기 때문에, `listen()` 함수나 `accept()` 함수의 호출을 통한 연결 설정의 과정이 불필요하다. 

또한 TCP 서버에서 클라이언트의 수만큼의 소켓이 필요했던 것과 달리 UDP는 하나의 소켓만 있으면 된다. 

이처럼 UDP 소켓은 연결 상태를 유지하지 않기 때문에, 데이터를 전송할 때마다 반드시 목적지의 주소를 함께 넣어줘야 한다. → `sendto()` 함수

그리고 데이터를 읽어들이는 경우는 UDP 패킷에 담겨온 발신지의 주소 정보를 함께 반환한다. → `recvfrom() (linux) / readfrom() (windows)` 함수

TCP 클라이언트 소켓은 `connect()` 함수 호출 시에 별도로 소켓에 주소를 바인딩하지 않아도 자동으로 IP 주소와 Port 번호가 할당이 되었다. 마찬가지로 UDP 소켓은 주소가 바인딩되어 있지 않을 경우,  `sendto()` 함수가 최초로 호출될 때 자동으로 IP 주소와 Port 번호가 할당된다. 

### TCP 패킷 vs UDP 데이터그램

TCP로 전송되는 데이터는 경계가 존재하지 않는다. 따라서 TCP 패킷은 전체 데이터의 일부분인 반면, UDP로 전송되는 데이터는 경계가 존재한다. 따라서 하나의 패킷이 하나의 데이터로 의미를 가진다. 이처럼 **의미 단위로 구분**되는 UDP의 데이터 패킷을 데이터그램이라고 한다. 

TCP 패킷을 전송할 때는 입출력 함수의 호출 횟수가 문제되지 않는다. 여러 번의 출력을 통해 보내진 데이터를 한 번의 입력을 통해 가져올 수도 있다. 반면, UDP 데이터그램은 반드시 입출력 함수의 호출 횟수가 동일해야 전체 데이터를 읽어들일 수 있다는 특징이 있다. 

### unconnected 소켓 vs connected 소켓

UDP 데이터의 전송 과정은 다음과 같이 표현할 수 있다. 

1. 소켓에 목적지 IP와 Port를 등록
2. 데이터 전송
3. 소켓에 등록된 목적지 정보 삭제 

이처럼 UDP 소켓은 목적지 정보가 계속 변경되기 때문에 하나의 소켓으로 여러 클라이언트에게 데이터를 전송할 수 있다. 그런데 하나의 클라이언트에게 오랜 시간 동안 여러 번 데이터를 전송할 경우에는 반복적으로 목적지 주소를 설정하고 리셋하는 작업이 불필요하다. 따라서 이런 경우에는 소켓에 한번 목적지를 등록해놓고 계속 사용하는 것이 효율적이다. 이런 경우 사용되는 것이 **connected 소켓**이다. 

unconnected 소켓은 목적지 정보가 등록되어 있지 않은 소켓이고, connected 소켓은 목적지 정보가 등록되어 있는 소켓을 가리킨다. connected 소켓을 만들기 위해서는 TCP 소켓에서 소켓을 연결하는 경우와 마찬가지로, `connect()` 함수를 호출해 목적지 주소를 등록한다. connected 소켓은 데이터 송수신 시 주소 등록이 필요한 `sendto()` 와 `readfrom()` 대신 TCP 소켓과 같은 입출력 함수(`send() / recv()`)를 사용할 수 있다는 이점이 있다.
