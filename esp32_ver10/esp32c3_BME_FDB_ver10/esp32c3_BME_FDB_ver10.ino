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
#define DHTPIN 3 
#define DHTTYPE DHT22  

String ele = "bme";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.stdtime.gov.tw");
DHT dht(DHTPIN, DHTTYPE);
// #define SEALEVELPRESSURE_HPA (1013.25)
// Adafruit_BME280 bme;

WebServer server(80);

String header;

const int rs = D2;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA" 
FirebaseData fbdo, fbdo_ALL, fbdo_D4;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = "";
String IP = "";
String STAT1 = "";
String STAT2 = "";
String CNSTAT = "";

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

  A = WiFi.macAddress();
  IP =  WiFi.localIP().toString();
  timeClient.begin();
  timeClient.setTimeOffset(28800);

  STAT1 = A + "/Temperature";
  STAT2 = A + "/Humidity";
  CNSTAT = A + "/esp";
  
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
  Firebase.RTDB.setStreamCallback(&fbdo_ALL, call, streamTimeoutCallback);
  Firebase.RTDB.setFloat(&fbdo_ALL, STAT1, dht.readTemperature());
  Firebase.RTDB.setFloat(&fbdo_ALL, STAT2, dht.readHumidity());
  // Firebase.RTDB.setFloat(&fbdo_ALL, STAT1, bme.readTemperature());
  // Firebase.RTDB.setFloat(&fbdo_ALL, STAT2, bme.readHumidity());
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");
  
  server.on("/info", handleINFO);
  server.onNotFound(handleNotFound);
  server.begin(); 
  Serial.println("ALL DONE");
}

void loop(){
  server.handleClient();
  time();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
    Serial.println("Refresh token");
  }

}
void streamTimeoutCallback(bool timeout){
  if (timeout){
    Serial.println("stream timeout, resuming...\n");
  }
  if (!fbdo_ALL.httpConnected()){
    Serial.printf("error code: %d, reason: %s\n\n", fbdo_ALL.httpCode(), fbdo_ALL.errorReason().c_str());
  } 
}
void handleINFO(void){
  server.send ( 200, "text/plain", "<h1>MAC=" + A + "</h1><h2>ELEMENT=" + ele + "</h2><h3>裝置名稱 = sMART sTUFF 溫溼度感測器</h3><h4>IP=" + IP + "</h4>" ); 
}
void handleNotFound(){                                 
  server.send(404, "text/plain", "404: Not found");   
}
void call(FirebaseStream data){
  if(Firebase.ready() && signupOK){
    if (fbdo_ALL.dataPath() == "/esp"){
        if(fbdo_ALL.dataType() == "string"){
          if (fbdo_ALL.stringData() == "1") {
            Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "2");
          }
        }
      }
    if (fbdo_ALL.dataPath() == "/Humidity"){
      Serial.print(fbdo_ALL.floatData());
      Serial.println(" % ");
    }
    if (fbdo_ALL.dataPath() == "/Temperature"){
      Serial.print(fbdo_ALL.floatData());
      Serial.println(" *C ");
    }
  }
}
void time() {
  timeClient.update();
  if (timeClient.getSeconds() == 0 || timeClient.getSeconds() == 15 || timeClient.getSeconds() == 30 || timeClient.getSeconds() == 45) {
    Firebase.RTDB.setFloat(&fbdo, STAT1, dht.readTemperature());
    Firebase.RTDB.setFloat(&fbdo, STAT2, dht.readHumidity());
    // Firebase.RTDB.setFloat(&fbdo, STAT1, bme.readTemperature());
    // Firebase.RTDB.setFloat(&fbdo, STAT2, bme.readHumidity());
  }
}