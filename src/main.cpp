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
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>


// WIFI CREDENTIALS
const char* ssid="kevin";
const char* password="12345678";
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);
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

void setupWifi()
{ 
  //pinMode(ledPin,OUTPUT);
  //digitalWrite(ledPin,HIGH);
  timeout = millis();
  Serial.println();
  Serial.print("Connecting...");
  Serial.println(ssid);
  wifiMulti.addAP(ssid,password);
  Serial.println();
  Serial.print("Connecting...");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(250);
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

  if (MDNS.begin("esp")) // Start the mDNS responder for esp.local
    Serial.println("mDNS responder started");
  else 
    Serial.println("Error setting up MDNS responder!");

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "<form action=\"/face\" method=\"POST\"><input type=\"submit\" value=\"FACE\"></form>");
  });
  server.on("/face", HTTP_POST, [](){
    server.send(200, "text/html", "<form action=\"/facesent\" method=\"POST\"><input type=\"text\" name=\"eye_angle\" placeholder=\"eye_angle\"></br><input type=\"text\" name=\"eye_distance\" placeholder=\"eye_distance\"><input type=\"text\" name=\"mouth_size\" placeholder=\"mouth_size\"><input type=\"text\" name=\"mouth_ypos\" placeholder=\"mouth_ypos\"><input type=\"text\" name=\"mouth_smile\" placeholder=\"mouth_smile\"><input type=\"submit\" value=\"SEND\"></form><p>(EYE_ANGLE, DIST_FROM_CENTER, MOUTH_SIZE, MOUTH_yPOS, MOUTH_SMILE)</p>");
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
    Eyes(function[0],function[1]);
    Mouth(function[2],function[3],-function[4]);
    for(int n = 0; n < 5; n++)
      Serial.println(function[n]);
    dis.sendBuffer();
    newFace = false;
  }
  
  // Check for serial messages and update display
  // if(getSerialMessage())    
  //   newFace = true;
}