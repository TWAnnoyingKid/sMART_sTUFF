// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h> 
#include <Wire.h>
#include <Adafruit_Sensor.h>
// #include <Adafruit_BME280.h>
#include <SPI.h>
#include <DHT.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define DHTPIN 10
#define DHTTYPE DHT22  

String ele = "bme";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.stdtime.gov.tw");
DHT dht(DHTPIN, DHTTYPE);
// #define SEALEVELPRESSURE_HPA (1013.25)
// Adafruit_BME280 bme;

WebServer server(80);

String header;
int rs = D9;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA" 
FirebaseData fbdo, fbdo_ALL;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = WiFi.macAddress();
String IP = "";
String Temp_Set = A + "/Temperature";
String Humid_Set = A + "/Humidity";
String CNSTAT = A + "/esp";
String h;
String t;

void setup() {
  Serial.begin(115200);
  
  pinMode(rs, INPUT_PULLUP);   digitalWrite(rs, HIGH);
  // bool status;
  // status = bme.begin(0x76); 
  dht.begin();
  
  WiFiManager wifiManager;
  if (digitalRead(rs) == LOW){
    wifiManager.resetSettings();
  }
  
  wifiManager.autoConnect("sMART sTUFF");
  IP =  WiFi.localIP().toString();
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

  if (!Firebase.RTDB.beginStream(&fbdo_ALL, A)) {
    Serial.print ("ALL begin error ");
    Serial.println(fbdo_ALL.errorReason());
  }
  Firebase.RTDB.setString(&fbdo_ALL, Temp_Set, "0");
  Firebase.RTDB.setString(&fbdo_ALL, Humid_Set, "0");
  // Firebase.RTDB.setFloat(&fbdo_ALL, Temp_Set, bme.readTemperature());
  // Firebase.RTDB.setFloat(&fbdo_ALL, Humid_Set, bme.readHumidity());
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");
  
  server.on("/info", handleINFO);
  server.onNotFound(handleNotFound);
  server.begin(); 
  Serial.println("ALL DONE");
}

void loop(){
  server.handleClient();
  time();
  ReadStat();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
    Serial.println("Refresh token");
  }

}
void handleINFO(void){
  server.send ( 200, "text/plain", "<h1>MAC=" + A + "</h1><h2>ELEMENT=" + ele + "</h2><h3>裝置名稱 = sMART sTUFF 溫溼度感測器</h3><h4>IP=" + IP + "</h4>" ); 
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
      if (fbdo_ALL.dataPath() == "/Humidity"){
        Serial.print(fbdo_ALL.stringData());
        Serial.println(" % ");
      }
      if (fbdo_ALL.dataPath() == "/Temperature"){
        Serial.print(fbdo_ALL.stringData());
        Serial.println(" *C ");
      }
    }
  }
}
void time() {
  timeClient.update();
  if (timeClient.getSeconds() == 0 || timeClient.getSeconds() == 15 || timeClient.getSeconds() == 30 || timeClient.getSeconds() == 45) {
    h = dht.readHumidity();
    t = dht.readTemperature();
    Serial.print("TEMP=");
    Serial.print(t);
    Serial.print("*C");
    Serial.print(" \\ HUMID=");
    Serial.print(h);
    Serial.println("%");
    Firebase.RTDB.setString(&fbdo_ALL, Temp_Set, t);
    Firebase.RTDB.setString(&fbdo_ALL, Humid_Set, h);
    // Firebase.RTDB.setFloat(&fbdo, Temp_Set, bme.readTemperature());
    // Firebase.RTDB.setFloat(&fbdo, Humid_Set, bme.readHumidity());
  }
  
}