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

String output1State = "on";
String output2State = "on";
String output3State = "on";

const int output1 = D1;
const int output2 = D2;
const int output3 = D3;
const int rs = D4;      //RESET WIFI
const int bt1 = A0;
const int bt2 = D0;
const int bt3 = D5;
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
int D1STAT = 0;
int D2STAT = 0;
int D3STAT = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(output1, OUTPUT);  digitalWrite(output1, LOW);
  pinMode(output2, OUTPUT);  digitalWrite(output2, LOW);
  pinMode(output3, OUTPUT);  digitalWrite(output3, LOW);
  pinMode(rs, INPUT);        digitalWrite(rs, HIGH);
  
  pinMode(bt1, INPUT_PULLUP);       digitalWrite(bt1, 1);
  pinMode(bt2, INPUT_PULLUP);       digitalWrite(bt2, 1);
  pinMode(bt3, INPUT_PULLUP);       digitalWrite(bt3, 1);
  
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
  qrcode.create(hs + " " + ele);
  
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

  server.begin();
}

void loop(){
  WiFiClient client = server.available();  

  STAT_Read();
  if (Firebase.RTDB.getString(&fbdo, STAT1)) {
    D1STAT = fbdo.stringData().toInt();
  }
  Relays(); 
  delay(300);
  
  STAT_Read();
  if (Firebase.RTDB.getString(&fbdo, STAT2)) {
    D2STAT = fbdo.stringData().toInt();
  }
  Relays(); 
  delay(300);

  STAT_Read();
  if (Firebase.RTDB.getString(&fbdo, STAT3)) {
    D3STAT = fbdo.stringData().toInt();
  }
  Relays();   
  delay(300);
}

void STAT_Read(){
  if(digitalRead(bt1) == 0){
    D1STAT = !D1STAT;
    Relays(); 
  }
  else if(digitalRead(bt2) == 0){
    D2STAT = !D2STAT;
    Relays(); 
  }
  else if(digitalRead(bt3) == 0){
    D3STAT = !D3STAT;
    Relays();
  }
}

void Relays(){  
  digitalWrite(output1, D1STAT);  
  digitalWrite(output2, D2STAT);
  digitalWrite(output3, D3STAT);
  if(D1STAT = 0) {
    Serial.println("D1 ON");
  }
  else if(D1STAT = 1) {
    Serial.println("D1 OFF");
  }
  if(D2STAT = 0) {
    Serial.println("D2 ON");
  }
  else if(D2STAT = 1) {
    Serial.println("D2 OFF");
  }
  if(D3STAT = 0) {
    Serial.println("D3 ON");
  }
  else if(D3STAT = 1) {
    Serial.println("D3 OFF");
  }

}
