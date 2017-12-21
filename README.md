# Temperature&Humidity Sensor with RaspberryPi
서버,클라이언트,라즈베리간 통신을 구현한 소스코드 (오픈소스 프로젝트 과목 과제)

 해당 프로그램은 제가 '오픈소스 프로젝트'라는 과목을 수강하면서 최종과제로 제출한 소스코드입니다.
 C언어 위주로 설계되었으며 상세한 구현사항은 아래와 같습니다.

[1] Server
 1) stems.h : 해당 소스코드들을 사용시에 도움을 주거나 Wrapper 함수가 포함된 코드가 있습니다.
 2) server.c : 서버입니다. 스레드 풀로 구현되어있으며, HTTP 프로토콜 양식을 받아서 요청을 받거나 처리합니다.
 3) request.h : 서버에게 요청받거나, 서버가 요청하는 사항을 처리해주기 위해 따로 분리한 모듈입니다.
 4) config-ws.txt : 서버의 포트번호나 큐 사이즈, 스레드의 갯수를 설정하기 위해 읽는 텍스트 파일입니다.
 5) clientGet.c : 클라이언트 부분입니다. HTTP프로토콜로는 GET요청을 하는 모듈입니다. 쉘로 구현되어있습니다.
 6) config-cg2.txt : clientGet을 실행할 때, 읽어오는 텍스트파일입니다.
 7) dataGet.c : 컴파일 시엔 dataGet.cgi 프로그램을 생성하며, 서버에서 DB와 연동하여,
		클라이언트에게 온습도 센서의 정보들을 받아오게 합니다.
 8) clientPost.c : 서버에게 HTTP프로토콜로 POST요청을 보내는 모듈입니다. 쉘로 구현되어있습니다.
		   차후엔 라즈베리파이의 clientRPI.c파일로 대체가능한 모듈입니다.
 9) config-cp.txt : clientPost가 읽어오는 텍스트파일입니다.
 10) dataPost.c : 컴파일 시엔 dataPost.cgi 프로그램을 생성하며, 서버에서 DB와 연동하여,
		  클라이언트에게 온습도 센서의 정보를 넣어주는 역할을 합니다.
 11) alarmClient.c : 알람 클라이언트입니다. 서버의 자식 프로세스로 실행되며, 일정 온도 혹은 습도가 넘어갈 시에,
		     파이프 통신(dataPost.cgi으로부터)으로 받아온 데이터를 통해 pushServer에 요청을 보냅니다.
 12) pushServer.c : clientGet의 자식 프로세스로 실행되는 프로그램입니다. alarmClient로부터 온 요청을 처리합니다.
 13) alarm.c : pushServer가 요청하면 해당 프로그램이 실행됩니다. 컴파일 시에 alarm.cgi로 실행파일을 생성합니다.

 [2] RaspberryPi
  1) clientRPI.c : 라즈베리파이에서 수행되는 프로그램이며, 실시간 온습도 센서의 값을 체크한 후 서버로 보내줍니다.


실행방법 : 
[1] Server Side
 - make
 - ./server     ## Server start
 - ./clientGet  ## clientGet start
 - ./clientPost ## clientPost start

   ** clientGet과 clientPost는 쉘 프로그램으로 구현되어 있습니다. 만약 명령어를 참고하고 싶을시엔 아래와 같이 사용합니다.
      clientGet의 도움말 : HELP, clientPost의 도움말 : help

[2] RaspberryPi Side
 - make
 - sudo ./clientRPI   ## if you are not root, you must input this command.