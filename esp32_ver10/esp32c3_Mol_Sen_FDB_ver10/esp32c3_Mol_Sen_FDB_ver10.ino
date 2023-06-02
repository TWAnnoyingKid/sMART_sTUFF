// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <Wire.h>

#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "moi";

WebServer server(80);

String header;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const int water_pin = 2;
const int rs = 3;
int RS; 

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA" 
FirebaseData fbdo, fbdo_ALL, fbdo_CT;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = WiFi.macAddress();
String IP = "";
const int dry = 790;
const int wet = 320;
int moi = 0;
int moipc=0;
int moipercent=0;
int HM=0;
int LM=0;
String WT = "";
String STAT1 = A + "/Moisture";
String WATER = A + "/WATER";
String CNSTAT = A + "/esp";;

void setup() {
  Serial.begin(115200);

  pinMode(water_pin, OUTPUT);  digitalWrite(water_pin, HIGH);
  pinMode(rs, INPUT);          digitalWrite(rs, HIGH);
  
  WiFiManager wifiManager;
  RS = digitalRead(rs);
  if (RS == LOW){
    wifiManager.resetSettings();
  }
  wifiManager.autoConnect("sMART sTUFF");
  IP = WiFi.localIP().toString();

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
  
  if  (!Firebase.RTDB.beginStream(&fbdo_ALL, A)){
    Serial.print ("D1 begin error ");
    Serial.println(fbdo_ALL.errorReason());
  }
  Firebase.RTDB.setString(&fbdo_ALL, WATER, "0");
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");

  server.on("/info", handleINFO);
  server.onNotFound(handleNotFound);
  server.begin(); 
  Serial.println("ALL DONE");
}

void loop(){
  time();
  set_moi();
  WATERING();
  server.handleClient();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void time(){
  timeClient.update();
  String a = "\\\"";
  String nowTime = a + timeClient.getHours() + ":" + timeClient.getMinutes() + a ;
  if (timeClient.getSeconds() > 0 & timeClient.getSeconds() < 2) {
    if(WT == nowTime){
      digitalWrite(water_pin, LOW);
    }
  }
  if (timeClient.getSeconds() > 6 & timeClient.getSeconds() < 8) {
    if(WT == nowTime){
      digitalWrite(water_pin, HIGH);
    }
  }
}
void set_moi(){
  moi = analogRead(A0);
  moipc = map(moi, dry, wet, 0, 100);  
  if (moipc>=100){ moipercent=100; }
  else if (moipc<=0){ moipercent=0; }
  else{ moipercent=moipc; }
  Firebase.RTDB.setString(&fbdo, STAT1, moipercent);
  delay(1000);

  if(Firebase.RTDB.getString(&fbdo, A + "/SetMoi/High")){
    HM=fbdo.stringData().toInt();
  }
  if(Firebase.RTDB.getString(&fbdo, A + "/SetMoi/Low")){
    LM=fbdo.stringData().toInt();
  }
  if ((HM > 0)&(LM >0)){
    if(moipercent < LM){
      Firebase.RTDB.setString(&fbdo_ALL, WATER, "1");  
    }
    else if(moipercent > HM){
      Firebase.RTDB.setString(&fbdo_ALL, WATER, "0"); 
    }
  }
}
void WATERING(){
  if(Firebase.ready() && signupOK){
    if  (!Firebase.RTDB.readStream(&fbdo_ALL)){
      Serial.print("D1 read error：");
      Serial.println(fbdo_ALL.errorReason());
    }
    if (fbdo_ALL.streamAvailable()) {
      if (fbdo_CT.dataPath() == "/TIME"){
        if(fbdo_CT.dataType() == "string"){
          WT = fbdo_CT.stringData();
          Serial.println("WT now is " + WT);
        } 
      }
      if (fbdo_ALL.dataPath() == "/esp"){
        if(fbdo_ALL.dataType() == "string"){
          if (fbdo_ALL.stringData() == "1") {
            Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "2");
          }
        }
      }
    }
  }
}
void handleINFO(void){
  server.send ( 200, "text/plain", "<h1>MAC=" + A + "</h1><h2>ELEMENT=" + ele + "</h2><h3>裝置名稱 = sMART sTUFF 三插座智慧延長線</h3><h4>IP=" + IP + "</h4>" ); 
}
void handleNotFound(){                                 
  server.send(404, "text/plain", "404: Not found");   
}