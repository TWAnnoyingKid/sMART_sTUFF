// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <Wire.h>
#include "SSD1306.h"
#include <qrcode.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "3stat";

SSD1306 display(0x3c, D6, D7);  //D6:SDA D7:SCL
QRcode qrcode (&display);

WiFiServer server(80);

String header;

String output1State = "off";
String output2State = "off";
String output3State = "off";

const int output1 = D1;
const int output2 = D2;
const int output3 = D3;
int rs = 14;
int RS; 

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA" 
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = "";
String hs = "";
String STAT1 = "";
String STAT2 = "";
String STAT3 = "";

void setup() {
  Serial.begin(115200);
  
  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  pinMode(rs, INPUT);
  digitalWrite(output1, LOW);
  digitalWrite(output2, LOW);
  digitalWrite(output3, LOW);
  digitalWrite(rs, HIGH);
  
  WiFiManager wifiManager;

  RS = digitalRead(rs);
  if (RS == LOW){
    wifiManager.resetSettings();
  }
  
  wifiManager.autoConnect("SmartStuff");
  A = WiFi.localIP().toString();
  hs = WiFi.localIP().toString();

  if (hs[3] == '.'){
    hs[3] = 'o'; 
  }  
  if (hs[7] == '.'){
    hs[7] = 'o'; 
  }  
  if (hs[9] == '.'){
    hs[9] = 'o'; 
  } 
  if (hs[10] == '.'){
    hs[10] = 'o'; 
  }   
  if (hs[11] == '.'){
    hs[11] = 'o'; 
  } 
  
  STAT1 = hs + "/D1";
  STAT2 = hs + "/D2";
  STAT3 = hs + "/D3";

  display.init();
  display.display();
  qrcode.init();
  qrcode.create(hs + " " + ele + " " + A);
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.RTDB.setString(&fbdo, STAT1, "1");
  Firebase.RTDB.setString(&fbdo, STAT2, "1");
  Firebase.RTDB.setString(&fbdo, STAT3, "1");

  server.begin();
}

void loop(){
  WiFiClient client = server.available();  
  
  if (Firebase.RTDB.getString(&fbdo, STAT1)) {
    if (fbdo.stringData() == "0") {
      Serial.println("D1 off");
      output1State = "off";
      digitalWrite(output1, HIGH);
    }
    else if (fbdo.stringData() == "1") {
      Serial.println("D1 on");
      output1State = "on";
      digitalWrite(output1, LOW);
    }
  }
  delay(10);

  if (Firebase.RTDB.getString(&fbdo, STAT2)) {
    if (fbdo.stringData() == "0") {
      Serial.println("D2 off");
      output1State = "off";
      digitalWrite(output2, HIGH);
    }
    else if (fbdo.stringData() == "1") {
      Serial.println("D2 on");
      output1State = "on";
      digitalWrite(output2, LOW);
    }
  }
  delay(10);

  if (Firebase.RTDB.getString(&fbdo, STAT3)) {
    if (fbdo.stringData() == "0") {
      Serial.println("D3 off");
      output1State = "off";
      digitalWrite(output3, HIGH);
    }
    else if (fbdo.stringData() == "1") {
      Serial.println("D3 on");
      output1State = "on";
      digitalWrite(output3, LOW);
    }
  }
  delay(10);

  if (client) {                           
    Serial.println("New Client.");       
    String currentLine = "";               
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {          
//        header = client.readStringUntil('\r');
        char c = client.read();           
        Serial.write(c);                   
       header += c;
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            Serial.println(header);
            
            if (header.indexOf("GET /1/on") >= 0) {
              Serial.println("D1 on");
              output1State = "on";
              digitalWrite(output1, LOW);
            } else if (header.indexOf("GET /1/off") >= 0) {
              Serial.println("D1 off");
              output1State = "off";
              digitalWrite(output1, HIGH);
            } else if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("D2 on");
              output2State = "on";
              digitalWrite(output2, LOW);
            } else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("D2 off");
              output2State = "off";
              digitalWrite(output2, HIGH);
            }else if (header.indexOf("GET /3/on") >= 0) {
              Serial.println("D3 on");
              output3State = "on";
              digitalWrite(output3, LOW);
            } else if (header.indexOf("GET /3/off") >= 0) {
              Serial.println("D3 off");
              output3State = "off";
              digitalWrite(output3, HIGH);
            }
            

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style>");
            client.println("</head>");
            

            client.println("<body><h1>IP=" + A + "</h1>");
            client.println("<body><h1>ELEMENT=" + ele + "</h1>");
            

            client.println("<p>D1 - State " + output1State + "</p>");
            if (output1State=="off") {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
 
            client.println("<p>D2 - State " + output2State + "</p>");
            if (output2State=="off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("<p>D3 - State " + output3State + "</p>");
            if (output3State=="off") {
              client.println("<p><a href=\"/3/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/3/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            
            client.println("</body></html>");
            
            client.println();
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
