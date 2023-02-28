// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>

#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

WiFiServer server(80);

String ele = "gog";
String header;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA" 
FirebaseData fbdo, fbdo_D1;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = "";
String STAT1 = "";
const int barking=D1;
const int Rlight=D2;
const int Llight=D5;
const int RlightRead=D3;
const int LlightRead=D6;

void setup() {
  Serial.begin(115200);
  pinMode(RlightRead, INPUT);
  pinMode(LlightRead, INPUT);
  pinMode(barking, OUTPUT);  digitalWrite(barking, LOW);
  pinMode(Rlight, OUTPUT);   digitalWrite(Rlight, LOW);
  pinMode(Llight, OUTPUT);   digitalWrite(Llight, LOW);
  
  WiFiManager wifiManager;
  
  wifiManager.autoConnect("SmartStuff");

  A = WiFi.macAddress();
  STAT1 = A + "/barking";

  timeClient.begin();
  timeClient.setTimeOffset(28800);
  
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
  Firebase.RTDB.setString(&fbdo_D1, STAT1, "0");
  if  (!Firebase.RTDB.beginStream(&fbdo_D1, STAT1)){
    Serial.printf("Barking begin error", fbdo_D1.errorReason().c_str());
  }

  server.begin();
  Serial.println("done"); 
}

void loop(){
  time();
  delay(1000);
  if((digitalRead(RlightRead)>0)&&(digitalRead(LlightRead)>0)){
    digitalWrite(Rlight, HIGH);
    digitalWrite(Llight, HIGH);
    digitalWrite(barking, HIGH);
    delay(500);
    digitalWrite(barking, LOW);   
  }

  if(Firebase.ready() && signupOK){
    if  (!Firebase.RTDB.readStream(&fbdo_D1)){
      Serial.printf("barking read error", fbdo_D1.errorReason().c_str());
    }
    if (fbdo_D1.streamAvailable()) {
      if (fbdo_D1.stringData() == "1") {
        for(int i = 0; i < 10; i++) {
          digitalWrite(barking, HIGH);
          Serial.println("barking");
          delay(500);
          digitalWrite(barking, LOW);   
          Serial.println("stop barking");
          delay(500);
        }
        Firebase.RTDB.setString(&fbdo_D1, STAT1, "0");        
      }
    }  
  }

  WEB();
}
void time(){
  timeClient.update(); 
}
void WEB(){
  WiFiClient client = server.available();
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

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset='utf-8' name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style>");
            client.println("</head>");
            

            client.println("<body><h1>MAC=" + A + "</h1>");
            client.println("<body><h2>ELEMENT=" + ele + "</h2>");
            client.println("<body><h3>裝置名稱 = GUIDING DOG</h3>");

            
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