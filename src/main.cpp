// TO-DO LIST /////////////////
// [----] STATE MACHINES
// [DONE] ANIMATION FUNCTIONS
// [----] TEXT/DRAW FUNCTIONS
// [----] FACIAL EXPRESSION PRESETS
// [DONE] SERIAL COMMUNICATIONz
// [DONE] STANDARD WIFI CONNECTION
// [----] ADVANCED WIFI CONNECTION
// [DONE] BASIC HTML IMPLEMENTATION
// [DONE] ADVANCED HTML IMPLEMENTATION
// [----] BASIC APP IMPLEMENTATION
///////////////////////////////

#include <Arduino.h>
//#include <U8x8lib.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>


// WIFI CREDENTIALS
const char* ssid="kevin";
const char* password="12345678";
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
uint8_t buzzerPin = D8;
uint8_t ledR = D7;
uint8_t ledG = D6;
uint8_t ledB = D5;

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

String MESSAGE;

bool newFace = false;
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


void setupFeedback()
{
  pinMode(buzzerPin,OUTPUT);
  pinMode(ledR,OUTPUT);
  pinMode(ledG,OUTPUT);
  pinMode(ledB,OUTPUT);
}
void giveFeedback(bool R = 0, bool G = 0, bool B = 0, int freq = 0)
{
    digitalWrite(ledR,R);
    digitalWrite(ledG,G); 
    digitalWrite(ledB,B);
    tone(buzzerPin,freq,50);    
}

void setupWifi()
{ 
  //digitalWrite(ledPin,HIGH);
  timeout = millis();
  Serial.println();
  Serial.print("Connecting...");
  Serial.println(ssid);
  wifiMulti.addAP(ssid,password);
  wifiMulti.addAP("kevinpc","esp123456");
  Serial.println();
  Serial.print("Connecting...");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    giveFeedback(1,0,0,0);
    delay(250);
    giveFeedback(0,0,0,200);
    Serial.print(".");
    if (millis() - timeout > 10000)
      break;
  }
  //digitalWrite(ledPin,LOW);
  Serial.println();
  if (WiFi.status() == WL_CONNECTED)
  {    
    giveFeedback(0,1,0,400);
    Serial.println("Wifi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
    delay(1000);
    giveFeedback();
  }
  else {
    Serial.println("Wifi NOT Connected");
    giveFeedback(1,0,0,100);
    delay(1000);
    giveFeedback();
  }
  if (MDNS.begin("esp8266")) {// Start the mDNS responder for esp.local
    Serial.println("mDNS responder started");
  }
  else {
    Serial.println("Error setting up MDNS responder!");
  }
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "<form action=\"/face\" method=\"POST\"><input type=\"submit\" value=\"FACE\"></form>");
  });
  server.on("/face", HTTP_POST, [](){
    server.send(200, "text/html", "</form><p>(EYE ANGLE, DIST FROM CENTER, MOUTH SIZE, MOUTH yPOS, MOUTH SMILE)</p></br><form action=\"/facesent\" method=\"POST\"><input type=\"text\" name=\"eye_angle\" placeholder=\"eye_angle\"><input type=\"text\" name=\"eye_distance\" placeholder=\"eye_distance\"></br><input type=\"text\" name=\"mouth_size\" placeholder=\"mouth_size\"><input type=\"text\" name=\"mouth_ypos\" placeholder=\"mouth_ypos\"><input type=\"text\" name=\"mouth_smile\" placeholder=\"mouth_smile\"></br><input type=\"submit\" value=\"SEND\">");
  });
  server.on("/facesent", HTTP_POST, [](){
    //DO SOMETHING WITH THE DATA
    function[0] = server.arg("eye_angle").toInt();
    function[1] = server.arg("eye_distance").toInt();
    function[2] = server.arg("mouth_size").toInt();
    function[3] = server.arg("mouth_ypos").toInt();
    function[4] = server.arg("mouth_smile").toInt();
    newFace = true;
    server.sendHeader("Location","/");
    server.send(303); 
  });
  server.onNotFound([](){
    server.send(404, "text/plain", "404: Not found");
  });

  server.begin();                            // Actually start the server
  Serial.println("HTTP server started");  

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

  return true;
}

void setup()
{
  Serial.begin(115200);
  setupDisplay();
  setupWifi();
  setupFeedback();
}

void loop() 
{
  // Listen for HTTP requests from clients
  server.handleClient();

  if(newFace)
  {
    dis.clearBuffer();
    // Format for input (EYES(2Param),MOUTH(3Param))
    // (EYE_ANGLE, DIST_FROM_CENTER, MOUTH_SIZE, MOUTH_yPOS, MOUTH_SMILE)
    giveFeedback(0,0,1,50);
    Eyes(function[0],function[1]);
    Mouth(function[2],function[3],-function[4]);
    for(int n = 0; n < 5; n++){
      Serial.println(function[n]);
    }
    dis.sendBuffer();
    delay(1000);
    giveFeedback();
    newFace = false;
    
  }
  
  // Check for serial messages and update display
  // if(getSerialMessage())    
  //   newFace = true;
}