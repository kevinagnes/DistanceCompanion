// TO-DO LIST
// [DONE] STATE MACHINES
// [DONE] ANIMATION FUNCTIONS
// [DONE] TEXT FUNCTION
// [DONE] DRAW FUNCTION
// [DONE] FACIAL EXPRESSION PRESETS
// [DONE] SERIAL COMMUNICATIONz
// [DONE] STANDARD WIFI CONNECTION
// [DONE] ADVANCED WIFI CONNECTION
// [DONE] BASIC HTML IMPLEMENTATION
// [DONE] ADVANCED HTML IMPLEMENTATION
// [DONE] BASIC APP IMPLEMENTATION

// HERE IS WHERE YOU CHOOSE BETWEEN 
// isAdafruit = true  => LIPO BATTERY
// isAdafruit = false => USB POWERED
#define isAdafruit false

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

// HTTPS //
#include "HTTPSRedirect.h"
#include "DebugMacros.h"

// OSC //
#include <OSCMessage.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const unsigned int localPort = 14587;        
OSCErrorCode error;

// HTTPS GOOGLE SPREADSHEET /////////
// Script ID from Google Scripts
const char *GScriptId = "AKfycbx3ElWQI0FU3mg4_pszULcIJ0gmrQyug2KR4NZKK7N6oUIzBKb1yb4L";
const int httpsPort = 443;
// Read from Google Spreadsheet string
String url3 = String("/macros/s/") + GScriptId + "/exec?read=A1";

HTTPSRedirect* client = nullptr;
const char* host = "script.google.com";
// Message that will be received form Google Spreadsheet
String myReceivedText = "";
// this is a flag to store the first char of the string
// and check if it changed before continuing
char oldHttpsMessage;

// used to store the values of free stack and heap
// before the HTTPSRedirect object is instantiated
// so that they can be written to Google sheets
// upon instantiation
unsigned int free_heap_before = 0;
unsigned int free_stack_before = 0;

// WIFI
ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

//Nokia 5110 Display Object
#if isAdafruit
U8G2_PCD8544_84X48_F_4W_SW_SPI dis(U8G2_R0, // #PIN for ADAFRUIT'S FEATHER HUZZAH
                       /* clock=*/   2,      // 2  CLK D4
                       /* data=*/   13,      // 13 DIN D3
                       /* cs=*/      5,      // 5  CE  D1
                       /* dc=*/      4,      // 4  DC  D2
                       /* reset=*/  12);     // 12 RST D6                        
uint8_t ledLCD = 14;                         // 14 LED D5
                       // connect D0 to RST // 16 
#else
U8G2_PCD8544_84X48_F_4W_SW_SPI dis(U8G2_R0, // #PIN for ADAFRUIT'S FEATHER HUZZAH
                       /* clock=*/  D4,      // 2  CLK D4
                       /* data=*/   D3,      // 13 DIN D3
                       /* cs=*/     D1,      // 5  CE  D1
                       /* dc=*/     D2,      // 4  DC  D2
                       /* reset=*/  D6);     // 12 RST D6                        
uint8_t ledLCD = D5;                         // 14 LED D5
#endif

// Get the Center of the display
int centerX = dis.getDisplayWidth()/2;
int centerY = dis.getDisplayHeight()/2;

// Timers 
long timer, timeout, httpsTimer;

// booleans for calling functions
bool newFace = false;
bool newTextOrDrawing = false;
bool online = false;

// array that holds the face parameters
int function[5];

// Turn on/off the LCD led
void backlight(bool OnOFF)
{
  timer = millis();
  #if isAdafruit 
  #else
  digitalWrite(ledLCD,!OnOFF);
  #endif
}
// FACE DRAWING FUNCTIONS
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
// ONLINE STATUS BAR DRAWING FUNCTION
void status()
{
  dis.setFont(u8g2_font_u8glib_4_tr);
  if (wifiMulti.run() != WL_CONNECTED)
  {
    dis.setDrawColor(0);
    dis.drawBox(0,0,dis.getWidth(),6);
  }
  else
  {
    dis.setDrawColor(255);
    dis.drawBox(0,0,dis.getWidth(),6);
    dis.setDrawColor(0);
    dis.drawStr(0,5,WiFi.SSID().c_str());
    // dis.setDrawColor(255);
    // dis.drawBox(0,0,dis.getWidth(),6);
    // dis.setDrawColor(0);
    // dis.drawStr(0,5,WiFi.localIP().toString().c_str());
  }
  dis.sendBuffer();
  dis.setFont(u8g2_font_5x7_tf);
  backlight(true);
  timer = millis();
}

// SETUP FUNCTIONS
void setupHttpsRedirect()
{
  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");  
  
  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }
}
void setupWifi()
{ 
  //digitalWrite(ledPin,HIGH);
  timeout = millis();
  Serial.println();
  Serial.print("Connecting...");
  wifiMulti.addAP("kevin","12345678");
  wifiMulti.addAP("kevinpc","esp123456");
  wifiMulti.addAP("GL-MT300N-V2-e59","goodlife");
  wifiMulti.addAP("ASK4 Wireless","");
  Serial.println();
  Serial.print("Connecting...");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    online = false;
    delay(250);
    Serial.print(".");
    if (millis() - timeout > 10000)
      break;
  }

  Serial.println();
  if (WiFi.status() == WL_CONNECTED)
  {    
    online = true;
    Serial.print("Connected to: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  }
  else {
    Serial.println("Wifi NOT Connected");
    //giveFeedback(1,0,0,100);
    
    timeout = millis();
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
void setupOSC()
{
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
}
void setupDisplay()
{
  #if isAdafruit
  #else
  dis.setDisplayRotation(U8G2_R2);
  #endif
  timer = millis();
  dis.begin();
  //dis.setFont(u8g2_font_ncenB08_tr);   // choose a suitable font
  dis.setFontDirection(0);
  dis.setFont(u8g2_font_5x7_tf);
  //dis.clearBuffer();
  Eyes(-75,8);
  Mouth(-20,-10,-1);
  dis.drawStr(10,45,"updating...");
  dis.sendBuffer();
}

// HELPER FUNCTIONS
// Resets the display
void resetDisplay()
{
    dis.setDrawColor(0);
    dis.drawBox(0,0,dis.getWidth(),dis.getHeight());
    dis.setDrawColor(255);
}
// Line breaker for text drawing
void formatMessage(String message, int col, int line, int maxcol, int maxline) {

int window_width = maxcol - col;
int window_height = maxline - line;

int font_width = dis.getMaxCharWidth();
int font_height = dis.getMaxCharHeight() + 1;

int amountline = round (window_height / font_height);
int amountchar = round (window_width / font_width);

String newmessage = "";
int sizeMessage = message.length() - 1;

int howMuchLine = round (sizeMessage / amountchar);
int index;
int lastLine = sizeMessage % amountchar; // Need another line?
int linePosition;

if (lastLine > 0) {
howMuchLine = howMuchLine + 1;
}

if (sizeMessage > amountchar) {
for (int l = 1; l <= howMuchLine; l++) {
linePosition = l;
for (int c = 1; c <= amountchar; c++) {
index = amountchar * (l - 1) + (c - 1);
if (index <= sizeMessage) {
newmessage = newmessage + message[index];
}
}
int lenNew = newmessage.length() + 1;
char splitted[lenNew];
newmessage.toCharArray(splitted, lenNew);
dis.drawUTF8 (col, (((font_height) * linePosition) + line) , splitted);
newmessage = "";
}
} else {
char splitted[message.length()+1];
message.toCharArray(splitted, message.length()+1);
dis.drawUTF8 (col, line , splitted);
}
}
int wordcount (String message) {
int startIndex = 0;
int countword = 0;
while (message.indexOf (" ", startIndex) != -1) {
startIndex = message.indexOf (" ", startIndex) + 1;
countword += 1;
}
countword += 1;
return (countword);
}
// Replace '_' to ' ' from received text
void newHTTPSText(String _input)
{
    myReceivedText = _input;
    myReceivedText.replace('_',' ');
    newTextOrDrawing = true;

}
// Convert received message to Face inputs
void newHTTPSFace(String _input)
{
  char *input = new char[_input.length() +1];
  strcpy(input, _input.c_str());
  char *separator = NULL;

  // Add the final 0 to end the C string
  input[_input.length()] = 0;
  byte index = 0;
  // Split the command in two values
  separator = strtok(input, ":");
  while (separator != NULL)
  {
    function[index] = atoi(separator);
    index++;
    separator = strtok(NULL, ":");
  }
  newFace = true;
}
// Convert received OSC message to Face inputs
void eye(OSCMessage &message) 
{
  function[0] = message.getInt(0);
  Serial.println(function[0]);
}
void eyed(OSCMessage &message) 
{
  function[1] = message.getInt(0);
  Serial.println(function[1]);
}
void mouth(OSCMessage &message) 
{
  function[2] = message.getInt(0);
  Serial.println(function[2]);
}
void mouthPos(OSCMessage &message) 
{
  function[3] = message.getInt(0);
  Serial.println(function[3]);
}
void smile(OSCMessage &message) 
{
  function[4] = message.getInt(0);
  Serial.println(function[4]);
}
void sender(OSCMessage &message)
{
  newFace = message.getBoolean(0);
  Serial.println(newFace);
}

// LOOP FUNCTIONS
// Get data from Google Sheets and calls apropriate functions
void httpsLoop()
{
  Serial.print("Connecting to ");
  Serial.println(host);
  
  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  static bool flag = false;

  if (connect_count > MAX_CONNECT){
    //error_count = 5;
    connect_count = 0;
    flag = false;
    delete client;
    return;
  }

  status();

  // Get Data from A1 Google Sheet
  if (client->GET(url3, host))
  {
    ++connect_count;
    String httpsMessage = client->getResponseBody();
    char thisChar = httpsMessage[0];
    // if first char is a number, received data is a FACE
    if (isDigit(thisChar)) 
    {
      newHTTPSFace(httpsMessage);
    }
    // if it's not a number, received data could be a text or a drawing
    else 
    {
      if (oldHttpsMessage == httpsMessage[1]) return;
      oldHttpsMessage = httpsMessage[1];
      newHTTPSText(httpsMessage);
    }
  }
  else{
    ++error_count;
    DPRINT("Error-count while connecting: ");
    DPRINTLN(error_count);
  }

  if (error_count > 3){
    Serial.println("Halting processor..."); 
    delete client;
    client = nullptr;
    Serial.printf("Final free heap: %u\n", ESP.getFreeHeap());
    Serial.printf("Final stack: %u\n", ESP.getFreeContStack());
    Serial.flush();
    //ESP.deepSleep(0);
  }
}
// Get data from OSC communication and calls apropriate functions
void OSCLoop()
{
  OSCMessage message;
  int size = Udp.parsePacket();

  if (size > 0) 
  {
    while (size--) 
    {
      message.fill(Udp.read());

      //Serial.print(Udp.read());
    }
    if (!message.hasError()) 
    {
      //Serial.println("iamhere");
      message.dispatch("/slider1", eye);
      message.dispatch("/slider2", eyed);
      message.dispatch("/slider3", mouth);
      message.dispatch("/slider4", mouthPos);
      message.dispatch("/slider5", smile);
      message.dispatch("/button1", sender);
      newFace = true;   
    } 
    else 
    {
      error = message.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}
// Light sleep for the not adafruit
void light_sleep(int time){
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  Serial.println(WiFi.status());
  delay(time);
  ESP.reset();
 }

void setup()
{
  Serial.begin(115200);
  pinMode(ledLCD,OUTPUT);
  digitalWrite(ledLCD,true);

  free_heap_before = ESP.getFreeHeap();
  free_stack_before = ESP.getFreeContStack();
  Serial.printf("Free heap: %u\n", free_heap_before);
  Serial.printf("Free stack: %u\n", free_stack_before);

  setupDisplay();
  setupWifi();
  //setupOSC();
  setupHttpsRedirect();
  httpsLoop();
}

void loop() 
{
  // Listen for HTTP requests from clients
  server.handleClient();

  // after 10s put the device to sleep
  if(millis() - timer > 10000)
  {
    dis.setDrawColor(0);
    dis.drawBox(0,0,dis.getWidth(),6);
    // dis.setDrawColor(1);
    // dis.drawBox(0,0,dis.getWidth(),dis.getHeight());
    dis.sendBuffer();
    backlight(false);

    // isAdafruit uses a LiPo battery
    // else uses a usb to power the device
    #if isAdafruit
    Serial.println("I'm going into deep sleep mode for 60 minutes");
    ESP.deepSleep(36e8);
    #else
    Serial.println("I'm going into deep sleep mode for 10 minutes");
    light_sleep(60e4);
    #endif
  }

  // if detects a new face draw the new face
  if(newFace)
  {
    Serial.println("newFace");
    resetDisplay();

    Eyes(function[0],function[1]);
    Mouth(function[2],function[3],-function[4]);
    for(int n = 0; n < 5; n++){
      Serial.println(function[n]);
    }

    status();

    newFace = false;
  }

  // if detects a new text or drawing output the data
  else if(newTextOrDrawing)
  {
    Serial.println("newTextOrDrawing");
    resetDisplay();

    // if first char is ',' it is a drawing
    if (myReceivedText[0] == ',')
    {
      char *str = (char*)myReceivedText.c_str();
      char* p = strtok(str, ",");
      const size_t bufferSize = 200;
      size_t index =0;
      int positions[bufferSize];
      while (p != nullptr && index < bufferSize) {
        positions[index++] = atoi(p);
        p = strtok(NULL,",");
      } 

      for (int i=0;i<index;i+=4)
      {
        dis.drawLine(positions[i],positions[i+1],positions[i+2],positions[i+3]);
      }
    }
    // if first char is not ',' it is a text
    else
    {
      dis.firstPage();
      do 
      {
        formatMessage(myReceivedText,0,6,84,48);
        delay(100);
      } while (dis.nextPage());
    }
    status();

    newTextOrDrawing = false;
  }

}


