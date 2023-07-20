                    
/*
 *  Ex_19 - WebSocket LED
 *
 *  Raspberry Pico-W Running WebSocket Server Interfacing to the
 *  LED on the TomatoCube Pico MainBoard
 *  
 *  LEDs
 *  --  GP20
 *  
 *  IR_RECV
 *  --  GP6
 *  
 *  NeoPixel
 *  -- GP20
 *
 *  I2C
 *  --  GP18 (SDA) 
 *  --  GP19 (SCL) 
 * 
 */

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <WebSocketsServer.h>
#include <WebServer.h>

WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);


#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <Adafruit_NeoPixel.h>

// I2C address
#define SR04_I2CADDR 0x57
#define OLED_I2CAADR 0x3C

SSD1306AsciiWire oled(Wire1);

#define I2C1_SDA  18
#define I2C1_SCL  19
#define LED       12
#define IR_RECV   6
#define NEO_PIXEL 20
#define LED_COUNT 3

#define MotorA_1 8
#define MotorA_2 9
#define MotorB_1 10
#define MotorB_2 11

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

bool ledState = 0;
unsigned long lastUltraSonicUpdate = 0;

unsigned int motorCommand = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Pico-W Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>TomatoCube - CockroachBot WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>LED - GP12</h2>
      <p class="state">state: <span id="state">-</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}:81`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "LED=1"){
      state = "ON";
    }
    else if (event.data == "LED=0"){
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
</script>
</body>
</html>
)rawliteral";


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

                // send message to client
                webSocket.sendTXT(num, "Connected");
                webSocket.sendTXT(num, ledState? "LED=1": "LED=0");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            if (strcmp((char*)payload, "toggle") == 0) {
              ledState = !ledState;
              webSocket.broadcastTXT(ledState? "LED=1": "LED=0");
            } else if (strcmp((char*)payload, "Forward") == 0) {
              Serial.printf("Forward\n");
              motorCommand = 1;
            } else if (strcmp((char*)payload, "Backward") == 0) {
              Serial.printf("Backward\n");
              motorCommand = 2;
            } else if (strcmp((char*)payload, "Left") == 0) {
              Serial.printf("Left\n");              
              motorCommand = 3;
            } else if (strcmp((char*)payload, "Right") == 0) {
              Serial.printf("Right\n");         
              motorCommand = 4;
            } else if (strcmp((char*)payload, "Stop") == 0) {
              Serial.printf("Stop\n");         
              motorCommand = 0;
            }

            break;
        case WStype_BIN:
        case WStype_ERROR:      
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
          break;
    }

}

// Code to read the range information from the UltraSonic sensor
int ping_mm()
{
    unsigned long distance = 0;
    byte i;   
    byte ds[3];
    long timeChecked;
    
    Wire1.beginTransmission(SR04_I2CADDR);
    Wire1.write(1);          //1 = cmd to start measurement.
    Wire1.endTransmission();

    delay(200); // Delay 200ms

    i = 0;
    Wire1.requestFrom(0x57,3);  //read distance       
    while (Wire1.available())
    {
     ds[i++] = Wire1.read();
    }        
    
    distance = (unsigned long)(ds[0] << 16);
    distance = distance + (unsigned long)(ds[1] << 8);
    distance = (distance + (unsigned long)(ds[2])) / 1000;
    //measured value between 10 mm (1 cm) to 6 meters (600 cm)
    if ((10 <= distance) && (6000 >= distance)) {
        return (int)distance;
    }
    else {
        return -1;
    }
}

// Direction 0: forward | 1: backward
// Speed = from 0 to 100
// Motor  0: Motor A | 1: MOtor B
void motorMoveControl(unsigned short motor, unsigned short direction, unsigned short speed) {    
            analogWrite((motor == 0) ? MotorA_1 : MotorB_1, (direction == 0) ? 0 : ((speed > 0) && (speed < 100)? speed + 150 : 0));
            analogWrite((motor == 0) ? MotorA_2 : MotorB_2, (direction == 1) ? 0 : ((speed > 0) && (speed < 100)? speed + 150 : 0));
}

void setup(){
  Wire1.setSDA(I2C1_SDA);
  Wire1.setSCL(I2C1_SCL);
  Wire1.begin(); 
  oled.begin(&Adafruit128x32, OLED_I2CAADR); 
  
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  delay(2000);

  Serial.begin(115200);

  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.println("WebSocket Server");
  
  WiFiMulti.addAP(ssid, password);
  
  while(WiFiMulti.run() != WL_CONNECTED) {
      delay(100);
  }
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  oled.println(WiFi.localIP());
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // handle index
  server.on("/", []() {
      // send index.html
      server.send(200, "text/html", index_html);
  });
  
  server.onNotFound(handleNotFound);

  server.begin();                                      
}

void loop() {
  webSocket.loop();
  server.handleClient();
  digitalWrite(LED, ledState);
  if (millis() >= lastUltraSonicUpdate + 1000) {
    char ultraBuffer[40];
    int dist = ping_mm();
    Serial.println(dist);
    sprintf(ultraBuffer, "Ultrasonic=%d", dist);
    webSocket.broadcastTXT(ultraBuffer);
    lastUltraSonicUpdate = millis();
    delay(2);//allow the cpu to switch to other tasks
  }
  else
    delay(2);//allow the cpu to switch to other tasks

  if (motorCommand == 0){    // STOP
      motorMoveControl(0, 0, 0);
      motorMoveControl(1, 0, 0);  
  }
  else if (motorCommand == 1){ // FORWARD
      motorMoveControl(0, 1, 20);
      motorMoveControl(1, 1, 20);  
  }
  else if (motorCommand == 2){ // BACKWARD
      motorMoveControl(0, 0, 20);
      motorMoveControl(1, 0, 20);  
  } 
  else if (motorCommand == 3){ // LEFT
      motorMoveControl(0, 0, 20);
      motorMoveControl(1, 1, 20); 
  }
  else if (motorCommand == 4){ // RIGHT
      motorMoveControl(0, 1, 20);
      motorMoveControl(1, 0, 20);  
  } 

}
                    
                
                    
                
