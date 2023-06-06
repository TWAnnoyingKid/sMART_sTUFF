// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "ledcontrol";
TaskHandle_t Task1;
TaskHandle_t Task2;

WebServer server(80);

String header;
const int LED_PIN = D9;
const int MODE_btn = D1;
const int COLOR_btn = D2;
const int TURNOFF_btn = D3;
const int rs = D7;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA"
#define USER_EMAIL "smart.stuff.18340@gmail.com"
#define USER_PASSWORD "Rayed18340"
FirebaseData fbdo, fbdo_ALL,;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = WiFi.macAddress();
String a = "\\\"";
String IP = "";
String controlby = "";
String controller_MAC = "";
String D1_TO = "";
String COLOR = A + "/COLOR";
// String CONTROLLER = A + "/CONTROLLER";
// String CONTROL = A + "/CONTROLBYCONTROLLER";
String MODE = A + "/MODE";
String CNSTAT = A + "/esp";
int base_color_set = 1;
int mode_set = 1;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);            digitalWrite(LED_PIN, HIGH);
  pinMode(rs, INPUT_PULLUP);           digitalWrite(rs, HIGH);
  pinMode(MODE_btn, INPUT_PULLUP);     digitalWrite(MODE_btn, HIGH);
  pinMode(COLOR_btn, INPUT_PULLUP);    digitalWrite(COLOR_btn, HIGH);
  pinMode(TURNOFF_btn, INPUT_PULLUP);  digitalWrite(TURNOFF_btn, HIGH);
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
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");
  Firebase.RTDB.setString(&fbdo_ALL, MODE, "1");

  server.on("/info", handleINFO);
  server.onNotFound(handleNotFound);
  server.begin(); 
  Serial.println("ALL DONE");
  
}

void loop() {
  server.handleClient();
  ReadStat();
  btn_press();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void handleINFO(void){
  server.send ( 200, "text/plain", "<h1>MAC=" + A + "</h1><h2>ELEMENT=" + ele + "</h2><h3>裝置名稱 = sMART sTUFF 智慧燈條控制器</h3><h4>IP=" + IP + "</h4>" ); 
}
void handleNotFound(){                                 
  server.send(404, "text/plain", "404: Not found");   
}

void ReadStat() {
  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.readStream(&fbdo_ALL)) {
      Serial.print("ALL read error：");
      Serial.println(fbdo_ALL.errorReason());
    }
    if (fbdo_ALL.streamAvailable()) {
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
void mode_btn_press(){
  if (digitalRead(MODE_btn) == LOW) { //改變燈條模式
    if (mode_set < 5){
      mode_set == mode_set + 1;
      delay(200);
    }
    else if (mode_set == 5){
      mode_set == 1;
      delay(200);
    }
  switch(mode_set){
    case 1:  //恆亮
      Firebase.RTDB.setString(&fbdo_ALL, MODE, "1");
      break;
    case 2:  //呼吸
      Firebase.RTDB.setString(&fbdo_ALL, MODE, "2");
      break;
    case 3:  //跑馬燈
      Firebase.RTDB.setString(&fbdo_ALL, MODE, "3");
      break;
    case 4:  //彩虹
      Firebase.RTDB.setString(&fbdo_ALL, MODE, "4");
      break;
    case 5:  //彩虹呼吸
      Firebase.RTDB.setString(&fbdo_ALL, MODE, "5");
      break;
    }
  }
}
void color_btn_press(){
  if (digitalRead(COLOR_btn) == LOW) { //改變顏色
    if (base_color_set < 7){
      base_color_set == base_color_set + 1;
      delay(200);
    }
    else if (base_color_set == 7){
      base_color_set == 1;
      delay(200);
    }
  }
  switch(base_color_set){
    case 1: //紅
      Serial.println(255,0,0);
      Firebase.RTDB.setString(&fbdo_ALL, COLOR, "255000000");
      break;
    case 2: //橙
      Serial.println(255,97,0);
      Firebase.RTDB.setString(&fbdo_ALL, COLOR, "255097000");
      break;
    case 3: //黃
      Serial.println(255,255,0);
      Firebase.RTDB.setString(&fbdo_ALL, COLOR, "255255000");
      break;
    case 4: //綠
      Serial.println(0,255,0);
      Firebase.RTDB.setString(&fbdo_ALL, COLOR, "000255000");
      break;
    case 5: //藍
      Serial.println(0,0,255);
      Firebase.RTDB.setString(&fbdo_ALL, COLOR, "000000255");
      break;
    case 6: //紫
      Serial.println(160,32,240);
      Firebase.RTDB.setString(&fbdo_ALL, COLOR, "160032240");
      break;
    case 7: //白
      Serial.println(255,255,255);
      Firebase.RTDB.setString(&fbdo_ALL, COLOR, "255255255");
      break;
    case 8: //黑
      Serial.println(0,0,0);
      Firebase.RTDB.setString(&fbdo_ALL, COLOR, "000000000");
      break;
  }
  delay(100);
}