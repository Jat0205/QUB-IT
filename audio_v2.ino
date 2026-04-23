#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include "DFRobotDFPlayerMini.h"

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))   // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/4, /*tx =*/5);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20 column and 4 rows
int i = 0;
void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();
#if (defined ESP32)
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/D3, /*tx =*/D2);
#else
  FPSerial.begin(9600);
#endif

  Serial.begin(115200);
  
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    
    while(true);
  }  
  myDFPlayer.setTimeOut(50000); //Set serial communictaion time out 500ms
  
  //----Set volume----
  myDFPlayer.volume(10);  //Set volume value (0~30).
  
  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  
  //----Mp3 play----
  /*myDFPlayer.play(1);  //Play the first mp3
  delay(10000);
  lcd.setCursor(0, 0);
  lcd.print("Initialize it!");
  myDFPlayer.play(2);  //Play the first mp3
  delay(10000);
  myDFPlayer.play(3);  //Play the first mp3
  delay(10000);
  myDFPlayer.play(4);  //Play the first mp3
  delay(10000);*/
  myDFPlayer.play(1);  //Play the first mp3
  delay(1000);
  int i = 0;
}

void loop()
{
  i++;
  myDFPlayer.play((i));  //Play the first mp3
  lcd.clear();
  lcd.print((i));
  delay(10000);
}