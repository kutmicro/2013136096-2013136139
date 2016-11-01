#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#define chipSelect 10
#if ARDUINO >= 100

int Button = 4;
int LED = 6;

// On Uno: camera TX connected to pin 2, camera RX to pin 3:
SoftwareSerial cameraconnection = SoftwareSerial(2, 3);

#else
NewSoftSerial cameraconnection = NewSoftSerial(2, 3);
#endif

Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

void setup() {
  pinMode(Button, INPUT);
  pinMode(LED, OUTPUT);

#if !defined(SOFTWARE_SPI)
  //#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  //if(chipSelect != 53) pinMode(53, OUTPUT);
#else
  if (chipSelect != 10) pinMode(10, OUTPUT);
#endif
  //#endif
  cameraconnection.begin(38400);
  Serial.begin(9600);
  Serial.println("VC0706 Camera snapshot test");

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }

  // Try to locate the camera
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  // Print out the camera version information (optional)
  char *reply = cam.getVersion();
  if (reply == 0) {
    Serial.print("Failed to get version");
  } else {
    Serial.println("----------------------");
    Serial.print(reply);
    Serial.println("----------------------");
  }

  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small

  uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");

  Serial.println("----------------------");
}


void loop() {
  if (digitalRead(Button) == HIGH)
  {
    Serial.println("Snap in 0.1 secs...");
    delay(100);
    digitalWrite(LED, HIGH);

    if (! cam.takePicture())
      Serial.println("Failed to snap!");
    else
      Serial.println("Picture taken!");
    delay(150);
    digitalWrite(LED, LOW);


    char filename[13];
    strcpy(filename, "MECHA00.JPG");
    for (int i = 0; i < 100; i++) {
      filename[5] = '0' + i / 10;
      filename[6] = '0' + i % 10;
      // create if does not exist, do not open existing, write, sync after write
      if (! SD.exists(filename)) {
        break;
      }
    }

    File imgFile = SD.open(filename, FILE_WRITE);

    uint16_t jpglen = cam.frameLength();
    Serial.print("Storing ");
    Serial.print(jpglen, DEC);
    Serial.print(" byte image.");

    int32_t time = millis();
    pinMode(8, OUTPUT);
    byte wCount = 0;
    while (jpglen > 0) {
      // read 32 bytes at a time;
      uint8_t *buffer;
      uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
      buffer = cam.readPicture(bytesToRead);
      imgFile.write(buffer, bytesToRead);
      if (++wCount >= 64) {
        Serial.print('.');
        wCount = 0;
      }
      jpglen -= bytesToRead;
    }
    imgFile.close();

    time = millis() - time;
    Serial.println("done!");
    Serial.print(time); Serial.println(" ms elapsed");
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
  }
}
