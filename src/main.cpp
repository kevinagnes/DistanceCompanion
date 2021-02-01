// TO-DO LIST /////////////////
// [DONE] ANIMATION FUNCTIONS
// [----] TEXT/DRAW FUNCTIONS
// [DONE] SERIAL COMMUNICATION
// [DONE] MOBILE WIFI CONNECTION
// [----] WIFI TO SERIAL COMM
// [----] APK/WEB INTEGRATION
///////////////////////////////

#include <Arduino.h>
//#include <U8x8lib.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

// WIFI CREDENTIALS
const char* ssid="kevin";
const char* password="12345678";
//int ledPin = LED_BUILTIN;

//OLED 128x64
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C dis(U8G2_R0,
//                              /* clock=*/ D1, 
//                              /* data=*/  D2, 
//                              /* reset=*/ U8X8_PIN_NONE);// without Reset
//Nokia 5110 Display
U8G2_PCD8544_84X48_F_4W_SW_SPI dis(U8G2_R0, 
                       /* clock=*/ D4, 
                       /* data=*/  D3, 
                       /* cs=*/    D1, 
                       /* dc=*/    D2, 
                       /* reset=*/ D0);  

int centerX = dis.getDisplayWidth()/2;
int centerY = dis.getDisplayHeight()/2;

int eyeMovementX = 0;
int eyeMovementY = 0;

long timer, timeout;

#define INPUT_SIZE 20
char serialRead;

bool updater = false;
//int function1,function2,function3,function4;
  int function[5];
void Eyes(int _deg, int _dist)
{
  
  int eyeL = centerX-15;
  int eyeR = centerX+15;
  int eyeY = centerY - 5;

  int x = sin(DEG_TO_RAD*_deg)*_dist;
  int y = cos(DEG_TO_RAD*_deg)*_dist;

  dis.drawDisc(eyeL + x,eyeY + y,4);
  dis.drawDisc(eyeR + x,eyeY + y,4);
  dis.drawCircle(eyeL,eyeY,12);
  dis.drawCircle(eyeR,eyeY,12);
  
}
void Mouth(int _size,int _yPos, int _smileSize)
{
  uint8_t mouthL = centerX - 15 - _size;
  uint8_t mouthR = centerX + 15 + _size;
  uint8_t mouthY = centerY + 20 + _yPos;
  uint8_t smile  = mouthY + _smileSize;
  
  dis.drawLine(mouthL,mouthY,mouthR,mouthY);
  dis.drawLine(mouthL,mouthY,mouthL,smile);
  dis.drawLine(mouthR,mouthY,mouthR,smile);
}

void setupWifi()
{ 
  //pinMode(ledPin,OUTPUT);
  //digitalWrite(ledPin,HIGH);
  timeout = millis();
  Serial.println();
  Serial.print("Connecting...");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  Serial.println();
  Serial.print("Connecting...");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (millis() - timeout > 10000)
      break;
  }
  //digitalWrite(ledPin,LOW);
  Serial.println();
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wifi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else 
    Serial.println("Wifi NOT Connected");
}
void setupDisplay()
{
  timer = millis();
  dis.begin();
  dis.clearBuffer();
  Eyes(0,0);
  Mouth(0,0,1);
  dis.sendBuffer();
}

bool getSerialMessage()
{
  /* REF CODE USED
     https://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string/20829 
     https://forum.arduino.cc/index.php?topic=450585.0 */
     
  // Get next command from Serial (add 1 for final 0)
  char input[INPUT_SIZE + 1];
  char *separator = NULL;
  byte size = Serial.readBytes(input, INPUT_SIZE);
  // Add the final 0 to end the C string
  input[size] = 0;
  byte index = 0;
  // Split the command in two values
  separator = strtok(input, ",");
  while (separator != NULL)
  {
    function[index] = atoi(separator);
    index++;
    separator = strtok(NULL, ",");
  }
  
  for(int n = 0; n < index; n++)
    Serial.println(function[n]);

  return true;
}

void setup()
{
  Serial.begin(115200);
  setupDisplay();
  setupWifi();
}

void loop() 
{
  // Check for serial messages and update display
  // Format for input (EYES(2Param),MOUTH(3Param))
  // (EYE_ANGLE, DIST_FROM_CENTER, MOUTH_SIZE, MOUTH_yPOS, MOUTH_SMILE)
  if(getSerialMessage())    
  {  
    dis.clearBuffer();
    Eyes(function[0],function[1]);
    Mouth(function[2],function[3],-function[4]);
    dis.sendBuffer();
  }
}