// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "3stat";
TaskHandle_t Task1;
TaskHandle_t Task2;

WebServer server(80);

String header;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.stdtime.gov.tw");

int output1 = 1;
int output2 = 2;
int output3 = 3;
int LED_PIN = 5;
int SW1 = 6;
int SW2 = 7;
int SW3 = 8;
int rs = 4;

String d1button = "non";
String d2button = "non";
String d3button = "non";
String D1_TC_TRI = "non";
String D2_TC_TRI = "non";
String D3_TC_TRI = "non";
String D1_TO_TRI = "non";
String D2_TO_TRI = "non";
String D3_TO_TRI = "non";

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
String D2_TC = "";
String D3_TC = "";
String D1_TO = "";
String D2_TO = "";
String D3_TO = "";
String STAT1 = A + "/D1";
String STAT2 = A + "/D2";
String STAT3 = A + "/D3";
String CNSTAT = A + "/esp";

String OnNum = "1";
String CloseNum = "0";

void setup() {
  Serial.begin(115200);

  pinMode(output1, OUTPUT);    digitalWrite(output1, LOW);
  pinMode(output2, OUTPUT);    digitalWrite(output2, LOW);
  pinMode(output3, OUTPUT);    digitalWrite(output3, LOW);
  
  pinMode(LED_PIN, OUTPUT);    digitalWrite(LED_PIN, HIGH);
  pinMode(rs, INPUT_PULLUP);   digitalWrite(rs, HIGH);
  
  pinMode(SW1, INPUT_PULLUP);  digitalWrite(SW1, HIGH);
  pinMode(SW2, INPUT_PULLUP);  digitalWrite(SW2, HIGH);
  pinMode(SW3, INPUT_PULLUP);  digitalWrite(SW3, HIGH);

  xTaskCreatePinnedToCore(
                    Task1code, 
                    "Task1",    
                    20000,     
                    NULL,      
                    0,      
                    &Task1, 
                    0);  
  
  WiFiManager wifiManager;
  // wifiManager.resetSettings();
  if (digitalRead(rs) == LOW) {
    wifiManager.resetSettings();
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
    Serial.print ("ALL begin error ");
    Serial.println(fbdo_ALL.errorReason());
  }
  
  Firebase.RTDB.setString(&fbdo_ALL, STAT1, OnNum);
  Firebase.RTDB.setString(&fbdo_ALL, STAT2, OnNum);
  Firebase.RTDB.setString(&fbdo_ALL, STAT3, OnNum);
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");

  if (Firebase.RTDB.getString(&fbdo_OT, A + "/OT/D1")) {
    D1_TO = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_OT, A + "/OT/D2")) {
    D2_TO = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_OT, A + "/OT/D3")) {
    D3_TO = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_CT, A + "/CT/D1")) {
    D1_TC = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_CT, A + "/CT/D2")) {
    D2_TC = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_CT, A + "/CT/D3")) {
    D3_TC = fbdo.stringData();
  }

  server.on("/info", handleINFO);
  server.onNotFound(handleNotFound);
  server.begin(); 
  Serial.println("ALL DONE");
}

void loop() {
  ReadStat();
  button_cahnge_stat();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
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
      if (fbdo_ALL.dataPath() == "/D2"){
        if (fbdo_ALL.stringData() == CloseNum) {
          digitalWrite(output2, HIGH);
          Serial.println("D2 OFF");
        } else if (fbdo_ALL.stringData() == OnNum) {
          digitalWrite(output2, LOW);
          Serial.println("D2 ON");
        }
      }
      if (fbdo_ALL.dataPath() == "/D3"){
        if (fbdo_ALL.stringData() == CloseNum) {
          digitalWrite(output3, HIGH);
          Serial.println("D3 OFF");
        } else if (fbdo_ALL.stringData() == OnNum) {
          digitalWrite(output3, LOW);
          Serial.println("D3 ON");
        }
      }
      if (fbdo_ALL.dataPath() == "/esp"){
        if(fbdo_ALL.dataType() == "string"){
          if (fbdo_ALL.stringData() == "1") {
            Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "2");
          }
        }
      }
      ///////////////////////////////////////////////////////////
      if (fbdo_ALL.dataPath() == "/OT/D1"){
        if(fbdo_ALL.dataType() == "string"){
          D1_TO = fbdo_ALL.stringData();
          Serial.println("D1_TO now is " + D1_TO);
        } 
      }
      if (fbdo_ALL.dataPath() == "/OT/D2"){
        if(fbdo_ALL.dataType() == "string"){
          D2_TO = fbdo_ALL.stringData();
          Serial.println("D2_TO now is " + D2_TO);
        } 
      }
      if (fbdo_ALL.dataPath() == "/OT/D3"){
        if(fbdo_ALL.dataType() == "string"){
          D3_TO = fbdo_ALL.stringData();
          Serial.println("D3_TO now is " + D3_TO);
        } 
      }
    ///////////////////////////////////////////////////////////
      if (fbdo_ALL.dataPath() == "/CT/D1"){
        if(fbdo_ALL.dataType() == "string"){
          D1_TC = fbdo_CT.stringData();
          Serial.println("D1_TC now is " + D1_TC);
        } 
      }
      if (fbdo_ALL.dataPath() == "/CT/D2"){
        if(fbdo_ALL.dataType() == "string"){
          D2_TC = fbdo_ALL.stringData();
          Serial.println("D2_TC now is " + D2_TC);
        } 
      }
      if (fbdo_ALL.dataPath() == "/CT/D3"){
        if(fbdo_ALL.dataType() == "string"){
          D3_TC = fbdo_ALL.stringData();
          Serial.println("D3_TC now is " + D3_TC);
        } 
      }
    }
  }
}
void button_cahnge_stat(){
    if(d1button != "non"){
      if (digitalRead(output1) == LOW) {
        Firebase.RTDB.setString(&fbdo_ALL, STAT1, OnNum);
        d1button = "non";
      } else if (digitalRead(output1) == HIGH) {
        Firebase.RTDB.setString(&fbdo_ALL, STAT1, CloseNum);
        d1button = "non";
      }
    }
    delay(100);
    if(d2button != "non"){
      if (digitalRead(output2) == LOW) {
        Firebase.RTDB.setString(&fbdo_ALL, STAT2, OnNum);
        d2button = "non";
      } else if (digitalRead(output2) == HIGH) {
        Firebase.RTDB.setString(&fbdo_ALL, STAT2, CloseNum);
        d2button = "non";
      }
    }
    delay(100);
    if(d3button != "non"){
      if (digitalRead(output3) == LOW) {
        Firebase.RTDB.setString(&fbdo_ALL, STAT3, OnNum);
        d3button = "non";
      } else if (digitalRead(output3) == HIGH) {
        Firebase.RTDB.setString(&fbdo_ALL, STAT3, CloseNum);
        d3button = "non";
      }
    }
    delay(100);
    if(D1_TO_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_ALL, STAT1, OnNum);
      D1_TO_TRI = "non";
    }
    if(D2_TO_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_ALL, STAT2, OnNum);
      D2_TO_TRI = "non";
    }
    if(D3_TO_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_ALL, STAT3, OnNum);
      D3_TO_TRI = "non";
    }
    delay(100);
    if(D1_TC_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_ALL, STAT1, CloseNum);
      D1_TC_TRI = "non";
    }
    if(D2_TC_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_ALL, STAT2, CloseNum);
      D2_TC_TRI = "non";
    }
    if(D3_TC_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_ALL, STAT3, CloseNum);
      D3_TC_TRI = "non";
    }
    delay(100);
}
void Task1code( void * pvParameters ){
  for(;;){
    server.handleClient();
    time();
    if (digitalRead(SW1) == LOW) {
      if (digitalRead(output1) == LOW) {
        digitalWrite(output1, HIGH);
        d1button = "true";
      } else if (digitalRead(output1) == HIGH) {
        digitalWrite(output1, LOW);
        d1button = "true";
      }
    }
    delay(100);
    if (digitalRead(SW2) == LOW) {
      if (digitalRead(output2) == LOW) {
        digitalWrite(output2, HIGH);
        d2button = "true";
      } else if (digitalRead(output2) == HIGH) {
        digitalWrite(output2, LOW);
        d2button = "true";
      }
    }
    delay(100);
    if (digitalRead(SW3) == LOW) {
      if (digitalRead(output3) == LOW) {
        digitalWrite(output3, HIGH);
        d3button = "true";
      } else if (digitalRead(output3) == HIGH) {
        digitalWrite(output3, LOW);
        d3button = "true";
      }
    }
    delay(100);
  } 
}
void time(){
  timeClient.update();
  String nowTime = a + timeClient.getHours() + ":" + timeClient.getMinutes() + a;
  // Serial.println(nowTime);
  if (timeClient.getSeconds() > 0 & timeClient.getSeconds() < 3) {
    if(D1_TO == nowTime){
      digitalWrite(output1, LOW);
      D1_TO_TRI = "TURE";
    }
    if(D2_TO == nowTime){
      digitalWrite(output2, LOW);
      D2_TO_TRI = "TURE";
    }
    if(D3_TO == nowTime){
      digitalWrite(output3, LOW);
      D3_TO_TRI = "TURE";
    }
    if(D1_TC == nowTime){
      digitalWrite(output1, HIGH);
      D1_TC_TRI = "TURE";
    }
    if(D2_TC == nowTime){
      digitalWrite(output2, HIGH);
      D2_TC_TRI = "TURE";
    }
    if(D3_TC == nowTime){
      digitalWrite(output3, HIGH);
      D3_TC_TRI = "TURE";
    }
  }
}