// https://www.grc.com/fingerprints.htm
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "3stat";

WiFiServer server(80);

String header;

String output1State = "off";
String output2State = "off";
String output3State = "off";

const int output1 = D1;
const int output2 = D2;
const int output3 = D3;
const int rs = D4;
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
  
  pinMode(output1, OUTPUT);    digitalWrite(output1, HIGH);
  pinMode(output2, OUTPUT);    digitalWrite(output2, HIGH);
  pinMode(output3, OUTPUT);    digitalWrite(output3, HIGH);
  pinMode(rs, INPUT_PULLUP);   digitalWrite(rs, HIGH);
  
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
  String fs = hs + "/FinalStat";
  
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
  Firebase.RTDB.setString(&fbdo, STAT1, "0");
  Firebase.RTDB.setString(&fbdo, STAT2, "0");
  Firebase.RTDB.setString(&fbdo, STAT3, "0");
  Firebase.RTDB.setString(&fbdo, fs, "重設狀態");

  server.begin();
}

void loop(){
  WiFiClient client = server.available();  
  
  if (Firebase.RTDB.getString(&fbdo, STAT1)) {
    if (fbdo.stringData() == "0") {
      output1State = "off";
    }
    else if (fbdo.stringData() == "1") {
      Serial.println("鐵捲門打開");
      output1State = "on";
      digitalWrite(output1, LOW);
      delay(200);
      digitalWrite(output1, HIGH);
      Firebase.RTDB.setString(&fbdo, STAT1, "0");
    }
  }

  if (Firebase.RTDB.getString(&fbdo, STAT2)) {
    if (fbdo.stringData() == "0") {
      output1State = "off";
    }
    else if (fbdo.stringData() == "1") {
      Serial.println("鐵捲門停");
      output1State = "on";
      digitalWrite(output2, LOW);
      delay(200);
      digitalWrite(output2, HIGH);
      Firebase.RTDB.setString(&fbdo, STAT2, "0");
    }
  }

  if (Firebase.RTDB.getString(&fbdo, STAT3)) {
    if (fbdo.stringData() == "0") {
      output1State = "off";
    }
    else if (fbdo.stringData() == "1") {
      Serial.println("鐵捲門關下");
      output1State = "on";
      digitalWrite(output3, LOW);
      delay(200);
      digitalWrite(output3, HIGH);
      Firebase.RTDB.setString(&fbdo, STAT3, "0");
    }
  }

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
              Firebase.RTDB.setString(&fbdo, STAT1, "1");
              Serial.println("D1 on");
              delay(200);
            }
            else if (header.indexOf("GET /2/on") >= 0) {
              Firebase.RTDB.setString(&fbdo, STAT2, "1");
              Serial.println("D2 on");
              delay(200);
            }
            else if (header.indexOf("GET /3/on") >= 0) {
              Firebase.RTDB.setString(&fbdo, STAT3, "1");
              Serial.println("D3 on");
              delay(200);
            }
            

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset='utf-8' name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style>");
            client.println("</head>");
            

            client.println("<body><h1>LO220鐵捲門開關</h1>");

            client.println("<p><a href=\"/1/on\"><button class=\"button\">UP</button></a></p>");
            client.println("<p><a href=\"/2/on\"><button class=\"button\">STOP</button></a></p>");
            client.println("<p><a href=\"/3/on\"><button class=\"button\">DOWN</button></a></p>");
            
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
