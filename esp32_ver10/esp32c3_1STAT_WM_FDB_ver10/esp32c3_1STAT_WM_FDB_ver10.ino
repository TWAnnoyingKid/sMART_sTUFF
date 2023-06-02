// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "1stat";
TaskHandle_t Task1;
TaskHandle_t Task2;

WebServer server(80);

String header;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.stdtime.gov.tw");

const int output1 = D10;
const int LED_PIN = D9;
const int SW1 = D8;
const int rs = D7;

String d1button = "non";
String D1_TC_TRI = "non";
String D1_TO_TRI = "non";

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA"
#define USER_EMAIL "smart.stuff.18340@gmail.com"
#define USER_PASSWORD "Rayed18340"
FirebaseData fbdo, fbdo_ALL, fbdo_CT, fbdo_OT;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = WiFi.macAddress();
String a = "\\\"";
String IP = "";
String D1_TC = "";
String D1_TO = "";
String STAT1 = A + "/D1";
String CNSTAT = A + "/esp";

String OnNum = "1";
String CloseNum = "0";

void setup() {
  Serial.begin(115200);

  pinMode(output1, OUTPUT);    digitalWrite(output1, LOW);
  pinMode(LED_PIN, OUTPUT);    digitalWrite(LED_PIN, HIGH);
  pinMode(rs, INPUT_PULLUP);   digitalWrite(rs, HIGH);
  pinMode(SW1, INPUT_PULLUP);  digitalWrite(SW1, HIGH);
  
  WiFiManager wifiManager;
  // wifiManager.resetSettings();
  if (digitalRead(rs) == LOW) {
    wifiManager.resetSettings();
    digitalWrite(LED_PIN, HIGH);
  }
  digitalWrite(LED_PIN, LOW);
  wifiManager.autoConnect("sMART sTUFF");

  A = WiFi.macAddress();
  IP = WiFi.localIP().toString();
  timeClient.begin();
  timeClient.setTimeOffset(28800);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo_ALL, A)) {
    Serial.print ("D1 begin error ");
    Serial.println(fbdo_ALL.errorReason());
  }
  Firebase.RTDB.setString(&fbdo_ALL, STAT1, OnNum);
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");
  if (Firebase.RTDB.getString(&fbdo_OT, A + "/OT/D1")) {
    D1_TO = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_CT, A + "/CT/D1")) {
    D1_TC = fbdo.stringData();
  }
  server.on("/info", handleINFO);
  server.onNotFound(handleNotFound);
  server.begin(); 
  Serial.println("ALL DONE");
  
}

void loop() {
  server.handleClient();
  time();
  ReadStat();
  if (digitalRead(SW1) == LOW) {
    if (digitalRead(output1) == LOW) {
      digitalWrite(output1, HIGH);
      Firebase.RTDB.setString(&fbdo, STAT1, CloseNum);
    } else if (digitalRead(output1) == HIGH) {
      digitalWrite(output1, LOW);
      Firebase.RTDB.setString(&fbdo, STAT1, OnNum);
    }
  }
  delay(100);

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void handleINFO(void){
  server.send ( 200, "text/plain", "<h1>MAC=" + A + "</h1><h2>ELEMENT=" + ele + "</h2><h3>裝置名稱 = sMART sTUFF 三插座智慧延長線</h3><h4>IP=" + IP + "</h4>" ); 
}
void handleNotFound(){                                 
  server.send(404, "text/plain", "404: Not found");   
}

void ReadStat() {
  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.readStream(&fbdo_ALL)) {
      Serial.print("D1 read error：");
      Serial.println(fbdo_ALL.errorReason());
    }
    if (fbdo_ALL.streamAvailable()) {
      if (fbdo_ALL.dataPath() == "/D1"){
        if (fbdo_ALL.stringData() == CloseNum) {
          digitalWrite(output1, HIGH);
          Serial.println("D1 OFF");
        } else if (fbdo_ALL.stringData() == OnNum) {
          digitalWrite(output1, LOW);
          Serial.println("D1 ON");
        }
      }
      if (fbdo_ALL.dataPath() == "/CT/D1"){
        if(fbdo_ALL.dataType() == "string"){
          D1_TC = fbdo_ALL.stringData();
          Serial.println("D1_TC now is " + D1_TC);
        } 
      }
      if (fbdo_ALL.dataPath() == "/OT/D1"){
        if(fbdo_ALL.dataType() == "string"){
          D1_TO = fbdo_ALL.stringData();
          Serial.println("D1_TO now is " + D1_TO);
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
void time(){
  timeClient.update();
  String nowTime = a + timeClient.getHours() + ":" + timeClient.getMinutes() + a;
  // Serial.println(nowTime);
  if (timeClient.getSeconds() > 0 & timeClient.getSeconds() < 3) {
    if(D1_TO == nowTime){
      digitalWrite(output1, LOW);
      Firebase.RTDB.setString(&fbdo, STAT1, OnNum);
    }
    if(D1_TC == nowTime){
      digitalWrite(output1, HIGH);
      Firebase.RTDB.setString(&fbdo, STAT1, CloseNum);
    }
  }
}