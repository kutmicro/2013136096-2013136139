#include <Wire.h>
#include "MutichannelGasSensor.h"

// CC3000 헤더파일,wifi실드를 사용하기 위한 라이브러리 연결
#include <Adafruit_CC3000.h>  
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"

// 카메라 VC0706 헤더파일
#include <Adafruit_VC0706.h>
#include <SD.h>


// CC3000 선언, SFE CC3000 쉴드를 사용을 위한 아두이노 보드의 핀 설정
#define ADAFRUIT_CC3000_IRQ   2       // interrupt 2번핀
#define ADAFRUIT_CC3000_VBAT  7       // WiFi Enable 7번핀
#define ADAFRUIT_CC3000_CS    10      // CS for CC3000 10번핀
// CC3000 객체 생성
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); 
#define WLAN_SSID       "myNetwork"   //네트워크의 SSID
#define WLAN_PASS       "myPassword"  //네트워크의 패스워드
#define WLAN_SECURITY   WLAN_SEC_WPA2 //무선 네트워크 보안설정
#define LISTEN_PORT           80      //TCP pory설정(HTTP protocol port 80)
#define MAX_ACTION            10      //HTTP action 최대길이
#define MAX_PATH              64      //HTTP action request path 최대길이
#define BUFFER_SIZE           MAX_ACTION + MAX_PATH + 20  //입력요청 정보를 위한 버퍼의 크기
#define TIMEOUT_MS            500     //입력요청 대기시간

Adafruit_CC3000_Server httpServer(LISTEN_PORT); //httpServer 초기화 , httpServer 객체생성
/*
uint8_t buffer[BUFFER_SIZE+1]; //입력요청 정보
int bufindex = 0;
char action[MAX_ACTION+1];
char path[MAX_PATH+1];
*/
// 카메라 선언
// 메가에서 spi통신 50~53번 핀을 사용
#define chipSelect 53 // 아두이노 SS 핀부분 : SD카드 모듈의 CS에 연결
Adafruit_VC0706 cam = Adafruit_VC0706(&Serial1); // 아두이노의 19(RX), 18(TX) 시리얼 포트 사용(시리얼 1이 19,18이 였음)
// 아두이노의 RX는 카메라모듈의 TX와 연결

void Camera();
void WebServer();
bool parseRequest(uint8_t* buf, int bufSize, char* action, char* path);
void parseFirstLine(char* line, char* action, char* path);
bool displayConnectionDetails(void);

//피에조
int piezo=8;//피에조 부저는 2번 핀으로 설정
int numTones=8;//numTones이라는 배열 수를 나타내는 변수
int tones[]={261,294,330,349,392,440,494,523};
//각각의 도레미파솔라시도

void setup()
{
    Serial.begin(9600);  // 시리얼 전송속도 9600
    Serial.println("powr on!");

    // 가스센서
    
    mutichannelGasSensor.begin(0x04); // I2C 통신을 위한 초기화, I2C주소 초기화 ?? 왜 0x04 일까 ??
    // 4번핀이 마스터, 나머지를 슬레이브??  , A5는 왜 초기화부분이 없는지 데이터가 아니라서?
    
    mutichannelGasSensor.powerOn(); // 가스 센서의 전원을 켠다.

    // CC3000
  Serial.println(F("\nInitializing..."));
  // 연결이 되지 않으면 무한루프
  if (!cc3000.begin())  //CC300시작, 연결상태 확인
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }

  // 연결이 되면 와이파이 아이디를 출력
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);

  // 와이파이 접속을 실패하면 무한루프
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) { //CC3000을 사용한 WIFI설정 (SSID, PW, 보안설정)
    Serial.println(F("Failed!"));
    while(1);
  }
   // 접속 성공 시 출력
  Serial.println(F("Connected!"));
  // IP가 할당 될 때 까지 DHCP process 를 계속 실행한다.
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) //DHCP check
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  
// wifi연결 상태를 시리얼 모니터에 출력한다.1111
  while (! displayConnectionDetails()) {    //IP주소 DNS, Gateway등 출력
    delay(1000);
  }
 
  httpServer.begin();   //http Server연결
  
  Serial.println(F("Listening for connections..."));

  // 카메라
   // SD카드 존재여부, 초기화 여부를 확인한다.
   if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }  
  // 카메라의 위치를 찾아낸다.
   if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }

 // 해상도 설정 부분
   cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small


  //피에조
    pinMode(piezo,OUTPUT); //피에저부조가 연결된 핀을 output으로 설정
}

void loop()
{
    float c;  // 가스의 ppm을 저장할 변수

    c = mutichannelGasSensor.measure_CO(); // 일산화탄소 측정
    Serial.print("The concentration of CO is ");
    // 측정된 값이 있으면 ppm을 출력
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
    
    // 담배연기가 감지되면 실행
    if( c > 100 ){ // 실험으로 바꿔야 하는 부분
      Camera(); // 사진 찍는 부분
      WebServer(); // 웹서버에 문구 출력

     // 일정음을 출력
       for(int i=0; i<numTones; i++){
           tone(piezo,tones[i]);

           delay(50);
       }
    noTone(piezo);
    
    }

    delay(30000);
    Serial.println("...");
}

void Camera(){
  // 사진 찍는 부분
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
    //파일이 어디까지 생성되어있는지 확인시켜줌
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  //쓰기형식으로 파일을 연다-->존재하지 않은 파일을 생성함
  //filename은 char*형이다. FILE_WRITE는 자료형 바이트
  File imgFile = SD.open(filename, FILE_WRITE);

  // jpglen(사진 위에서 찍은 것->IMAGE00)의 크기를 구함(바이트)
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");//크기가 어느정도인지 출력(시리얼모니터로 확인)

 // pinMode(8, OUTPUT);//8번핀을 OUTPUT으로 줌
  // Read all the data up to # bytes!
  byte wCount = 0; //디버깅 용
  while (jpglen > 0) {//jpglen이 양수일 때->아직 메모리들이 안옴겨진 것 
    uint8_t *buffer;  //임시저장공간 생성
    uint8_t bytesToRead = min(32, jpglen); //32씩 읽다가 jpglen의 크기가 32바이트보다 작다면(남은크기) 그것만큼 읽음
    buffer = cam.readPicture(bytesToRead);//위의 크기만큼 사진을 읽어들이고 버퍼에 저장
    imgFile.write(buffer, bytesToRead);//파일을 옴기는 것
    if(++wCount >= 64) { //돌아가고있는지 확인하기 위함
      Serial.print('.');
      wCount = 0;
    }
    jpglen -= bytesToRead;//옴긴만큼 빼줌
  }
  imgFile.close();//파일을 닫아줌
 
  
  Serial.println("done!");
}

// WiFi
void WebServer(){
  Adafruit_CC3000_ClientRef client = httpServer.available();  
  if (client) {   //client가 연결되었는지 검사
    Serial.println(F("Client connected."));
    boolean currentLineIsBlank = true;   // http요청의 빈 라인 검사용 변수

    while(client.connected()){
      if(client.available()){//데이터 입력이 있는지 검사
        char c = client.read();//입력데이터 읽음
        Serial.write(c);//데이터가 있으면 읽어서 저장
        if(c == '\n' && currentLineIsBlank){//데이터가 빈줄이면
        client.println("HTTP/1.1 200 OK"); // 이 출력문구들을 실행(html문법), HTML문서의 시작
        client.println("Content-Type: text/plain");//어디에 출력된게 아니라 문서의 모든 내용을 담고있는 것
        client.println("Connection: close");
        client.println("Server: Adafruit CC3000"); 
        client.println("Refresh: 5");
        client.println();
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");//
        /////////////////////////////////////////////////////////////////////////
        for(int analogChannel = 0; analogChannel < 6; analogChannel++){//우리 입맛대로 변경 가능
          int sensorReading = analogRead(analogChannel);
          client.print("아날로그 입력 은: ");//문구 출력(바디 부분, 본문 내용)
          client.print(analogChannel);//
          client.print(" is ");
          client.print(sensorReading);
          client.print("<br />");//break라는 뜻으로 줄바꿈 기능
        }/*
        
          client.print("<head>");//헤더시작(사이엔 사이트 제목)
          client.print("</head>");//헤더 끝
          client.print("<body>");//안에는 본문내용
          client.print("</body>");
        */
        ////////////////////////////////////////////////////////////////////////////////
        client.println("</html>");//문서의 끝을 알림
        break;
        }
        if(c=='\n'){//빈라인이면 새라인 시작
            currentLineIsBlank=true;//ture나 false로 위의 if문을 제어
        }
        else if(c!='\r'){//빈라인이 아니면 현재 라인에서 문자를 읽음
            currentLineIsBlank=false;
        }
      }//if(client.available())끝
  }//while(client.connected()) 끝
  delay(1);//웹 브라우저 전송 위해 대기
  client.stop();//클라이언트 연결을 닫음
   Serial.println("client disonnected");
  }//if(client) 끝
}

// wifi연결 상태를 시리얼 모니터에 출력하기 위한 함수
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
