# Temperature&Humidity Sensor with RaspberryPi
> 서버,클라이언트,라즈베리파이 간의 통신을 구현한 시스템
> C언어 위주로 설계되었으며 상세한 구현사항은 아래와 같습니다.

## 개요

HTTP 프로토콜을 기반으로 서버와 클라이언트 간의 통신이 이루어집니다. IoT 온습도 센서를 주고받고 필요에 따라 푸쉬알림도 제공합니다.
- Server: 서버는 온습도 데이터를 수신/발신합니다. 온습도 데이터를 받은 후 MySQL에 저장합니다.
- Client(clientRPI): 라즈베리파이 쪽의 클라이언트이며, 온습도 센서의 데이터를 서버에게 발신합니다.
- Client(clientGet): 서버로부터 데이터를 요구하는 클라이언트입니다. shell 형태로 인터페이스가 제공되고 있으며, 명령어에 따라 서버에게 데이터를 요구하게 됩니다.

## 역할분담 (2인 1조)
- 이건희 : 테스트 및 배포환경 구축, 메인 서버 및 푸쉬, clientGet 구현, DB 관리, 모듈단위 테스트
- 남지현 : 클라이언트(clientPost) 및 클라이언트 단위 푸쉬 구현, 라즈베리파이 담당, 전반적인 테스트

## 개발 환경,언어,기술스택
- 메인 언어: C언어
- 개발환경 : Ubuntu 16.04 LTS
- 기술스택: MySQL, shell, Raspberry-Pi
- 그 외의 스킬: Thread-pool, Using HTTP Protocol, DTH Sensor

## Server
- stems.h : 해당 소스코드들을 사용시에 도움을 주거나 Wrapper 함수가 포함된 코드가 있습니다.
- server.c : 서버입니다. 스레드 풀로 구현되어있으며, HTTP 프로토콜 양식을 받아서 요청을 받거나 처리합니다.
- request.h : 서버에게 요청받거나, 서버가 요청하는 사항을 처리해주기 위해 따로 분리한 모듈입니다.
- config-ws.txt : 서버의 포트번호나 큐 사이즈, 스레드의 갯수를 설정하기 위해 읽는 텍스트 파일입니다.
- dataGet.c : 컴파일 시엔 dataGet.cgi 프로그램을 생성하며, 서버에서 DB와 연동하여,
		클라이언트에게 온습도 센서의 정보들을 받아오게 합니다.
- dataPost.c : 컴파일 시엔 dataPost.cgi 프로그램을 생성하며, 서버에서 DB와 연동하여,
		  클라이언트에게 온습도 센서의 정보를 넣어주는 역할을 합니다.
- alarmClient.c : 알람 클라이언트입니다. 서버의 자식 프로세스로 실행되며, 일정 온도 혹은 습도가 넘어갈 시에,
		     파이프 통신(dataPost.cgi으로부터)으로 받아온 데이터를 통해 pushServer에 요청을 보냅니다.
 
 ## Client (clientGet)
 - clientGet.c : 클라이언트 부분입니다. HTTP프로토콜로는 GET요청을 하는 모듈입니다. 쉘로 구현되어있습니다.
 - config-cg2.txt : clientGet을 실행할 때, 읽어오는 텍스트파일입니다.
 - pushServer.c : clientGet의 자식 프로세스로 실행되는 프로그램입니다. alarmClient로부터 온 요청을 처리합니다.
- alarm.c : pushServer가 요청하면 해당 프로그램이 실행됩니다. 컴파일 시에 alarm.cgi로 실행파일을 생성합니다.

 ## RaspberryPi (clientPost 대체)
 - clientRPI.c : 라즈베리파이에서 수행되는 프로그램이며, 실시간 온습도 센서의 값을 체크한 후 서버로 보내줍니다.
 - config-cp.txt : clientPost가 읽어오는 텍스트파일입니다.
 - clientPost.c : 서버에게 HTTP프로토콜로 POST요청을 보내는 모듈입니다. 쉘로 구현되어있습니다.
		   차후엔 라즈베리파이의 clientRPI.c파일로 대체가능한 모듈입니다.


## 실행방법 : 
- Server Side
```
make
./server     ## Server start
./clientGet  ## clientGet start
./clientPost ## clientPost start
```
   
   ** clientGet과 clientPost는 쉘 프로그램으로 구현되어 있습니다. 만약 명령어를 참고하고 싶을시엔 아래와 같이 사용합니다.
      clientGet의 도움말 : HELP, clientPost의 도움말 : help

[2] RaspberryPi Side
```
make
sudo ./clientRPI   ## if you are not root, you must input this command.
```
