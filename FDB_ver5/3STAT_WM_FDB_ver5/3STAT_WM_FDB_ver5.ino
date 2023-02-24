// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "3stat";

WiFiServer server(80);

String header;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

String output1State = "on";
String output2State = "on";
String output3State = "on";

const int output1 = D1;
const int output2 = D2;
const int output3 = D3;
const int SW1 = D5;
const int SW2 = D6;
const int SW3 = D7;
const int rs = D0;
const int wifi_led = D8;
int RS; 

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA" 
FirebaseData fbdo, fbdo_D1, fbdo_D2, fbdo_D3;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = "";
String STAT1 = "";
String STAT2 = "";
String STAT3 = "";

void setup() {
  Serial.begin(115200);
  
  pinMode(output1, OUTPUT);    digitalWrite(output1, LOW);
  pinMode(output2, OUTPUT);    digitalWrite(output2, LOW);
  pinMode(output3, OUTPUT);    digitalWrite(output3, LOW);
  pinMode(wifi_led, OUTPUT);   digitalWrite(wifi_led, HIGH);
  pinMode(rs, INPUT_PULLUP);
  pinMode(SW1, INPUT_PULLUP);  digitalWrite(SW1, HIGH); 
  pinMode(SW2, INPUT_PULLUP);  digitalWrite(SW2, HIGH); 
  pinMode(SW3, INPUT_PULLUP);  digitalWrite(SW3, HIGH); 
  WiFiManager wifiManager;
  // wifiManager.resetSettings();
  RS = digitalRead(rs);
  if (RS == LOW){
    wifiManager.resetSettings();
    OUT();  
  }
  OUT();  
  if(WiFi.getMode()==0){
    OUT_NF();  
  }
  wifiManager.autoConnect("sMART sTUFF");
  OUT();  
  A = WiFi.macAddress();

  timeClient.begin();
  timeClient.setTimeOffset(28800);
  
  STAT1 = A + "/D1";
  STAT2 = A + "/D2";
  STAT3 = A + "/D3";
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  OUT();  
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  OUT();  
  
  Firebase.RTDB.setString(&fbdo_D1, STAT1, "1");
  OUT();  
  Firebase.RTDB.setString(&fbdo_D2, STAT2, "1");
  OUT();  
  Firebase.RTDB.setString(&fbdo_D3, STAT3, "1");
  OUT();  
  if  (!Firebase.RTDB.beginStream(&fbdo_D1, STAT1)){
    Serial.printf("D1 begin error", fbdo_D1.errorReason().c_str());
  }
  OUT();  
  if  (!Firebase.RTDB.beginStream(&fbdo_D2, STAT2)){
    Serial.printf("D2 begin error", fbdo_D2.errorReason().c_str());
  }
  OUT();  
  if  (!Firebase.RTDB.beginStream(&fbdo_D3, STAT3)){
    Serial.printf("D3 begin error", fbdo_D3.errorReason().c_str());
  }
  OUT();  
  digitalWrite(wifi_led, LOW); 
  server.begin();
}

void loop(){
  OUT();
  timeClient.update();
  OUT();
  time();
  OUT();
  ReadStat();
  OUT();
  WEB();
  OUT();
}
void time(){
  OUT();
  String a = "\\\"";
  String nowTime = a + timeClient.getHours() + ":" + timeClient.getMinutes() + a ;
  // Serial.println(timeClient.getSeconds());
  delay(200);
  OUT();
  if(timeClient.getSeconds()<8 & timeClient.getSeconds()>0){
    OUT();
    if (Firebase.RTDB.getString(&fbdo, A+"/OT/D1")) {
      if (fbdo.stringData() == nowTime) {
        Firebase.RTDB.setString(&fbdo_D1, STAT1, "1");    
      }
    }
    OUT();
    if (Firebase.RTDB.getString(&fbdo, A+"/CT/D1")) {
      if (fbdo.stringData() == nowTime) {
       Firebase.RTDB.setString(&fbdo_D1, STAT1, "0");
      }
    }
    OUT();
    if (Firebase.RTDB.getString(&fbdo, A+"/OT/D2")) {
      if (fbdo.stringData() == nowTime) {
       Firebase.RTDB.setString(&fbdo_D2, STAT2, "1");    
      }
    }
    OUT();
    if (Firebase.RTDB.getString(&fbdo, A+"/CT/D2")) {
      if (fbdo.stringData() == nowTime) {
       Firebase.RTDB.setString(&fbdo_D2, STAT2, "0");
      }
    }  
    OUT();
    if (Firebase.RTDB.getString(&fbdo, A+"/OT/D3")) {
      if (fbdo.stringData() == nowTime) {
       Firebase.RTDB.setString(&fbdo_D3, STAT3, "1");    
      }
    } 
    OUT();
    if (Firebase.RTDB.getString(&fbdo, A+"/CT/D3")) {
      if (fbdo.stringData() == nowTime) {
        Firebase.RTDB.setString(&fbdo_D3, STAT3, "0");
      }
    }
    OUT();
  }  
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
            client.println("<body><h3>裝置名稱 = sMART sTUFF 三插座智慧延長線</h3>");
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
void OUT() {
  if(digitalRead(SW1)==LOW){
    if(digitalRead(output1)==LOW){
      digitalWrite(output1, HIGH);
      Firebase.RTDB.setString(&fbdo_D1, STAT1, "0");      
    }
    else if(digitalRead(output1)==HIGH){
      digitalWrite(output1, LOW);
      Firebase.RTDB.setString(&fbdo_D1, STAT1, "1");
    }
  }
  if(digitalRead(SW2)==LOW){
    if(digitalRead(output2)==LOW){
      digitalWrite(output2, HIGH);
      Firebase.RTDB.setString(&fbdo_D2, STAT2, "0");
    }
    else if(digitalRead(output2)==HIGH){
      digitalWrite(output2, LOW);
      Firebase.RTDB.setString(&fbdo_D2, STAT2, "1");
    }
  }
  if(digitalRead(SW3)==LOW){
    if(digitalRead(output3)==LOW){
      digitalWrite(output3, HIGH);
      Firebase.RTDB.setString(&fbdo_D3, STAT3, "0");
    }
    else if(digitalRead(output3)==HIGH){
      digitalWrite(output3, LOW);
      Firebase.RTDB.setString(&fbdo_D3, STAT3, "1");
    }
  }
}
void ReadStat(){
  OUT(); 
  if(Firebase.ready() && signupOK){
    OUT();
    if  (!Firebase.RTDB.readStream(&fbdo_D1)){
      Serial.printf("D1 read error", fbdo_D1.errorReason().c_str());
    }
    OUT();
    if (fbdo_D1.streamAvailable()) {
      OUT();
      if (fbdo_D1.stringData() == "0") {
        Serial.println("D1 off");
        output1State = "off";
        digitalWrite(output1, HIGH);
      }
      else if (fbdo_D1.stringData() == "1") {
        Serial.println("D1 on");
        output1State = "on";
        digitalWrite(output1, LOW);
      } 
    }  
  } 
  OUT();
  if(Firebase.ready() && signupOK){
    OUT();
    if  (!Firebase.RTDB.readStream(&fbdo_D2)){
      Serial.printf("D2 read error", fbdo_D2.errorReason().c_str());
    }
    OUT();
    if (fbdo_D2.streamAvailable()) {
      OUT();
      if (fbdo_D2.stringData() == "0") {
        Serial.println("D2 off");
        output2State = "off";
        digitalWrite(output2, HIGH);
      }
      else if (fbdo_D2.stringData() == "1") {
        Serial.println("D2 on");
        output2State = "on";
        digitalWrite(output2, LOW);
      }
    }
  }
  OUT();
  if(Firebase.ready() && signupOK){
    OUT();
    if  (!Firebase.RTDB.readStream(&fbdo_D3)){
       Serial.printf("D3 read error", fbdo_D3.errorReason().c_str());
    }  
    OUT();
    if (fbdo_D3.streamAvailable()) {
      OUT();
      if (fbdo_D3.stringData() == "0") {
        Serial.println("D3 off");
        output3State = "off";
        digitalWrite(output3, HIGH);
      }
      else if (fbdo_D3.stringData() == "1") {
        Serial.println("D3 on");
        output3State = "on";
        digitalWrite(output3, LOW);
      }    
    }
  }
  OUT();
}
void OUT_NF() {
  if(digitalRead(SW1)==LOW){
    if(digitalRead(output1)==LOW){
      digitalWrite(output1, HIGH); 
    }
    else if(digitalRead(output1)==HIGH){
      digitalWrite(output1, LOW);
    }
  }
  if(digitalRead(SW2)==LOW){
    if(digitalRead(output2)==LOW){
      digitalWrite(output2, HIGH);
    }
    else if(digitalRead(output2)==HIGH){
      digitalWrite(output2, LOW);
    }
  }
  if(digitalRead(SW3)==LOW){
    if(digitalRead(output3)==LOW){
      digitalWrite(output3, HIGH);
    }
    else if(digitalRead(output3)==HIGH){
      digitalWrite(output3, LOW);
    }
  }
}