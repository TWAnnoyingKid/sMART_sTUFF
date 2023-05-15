// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "bme";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.stdtime.gov.tw");

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

WiFiServer server(80);

String header;

const int rs = D4;
int RS; 

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
  
  pinMode(rs, INPUT);
  digitalWrite(rs, HIGH);
  bool status;
  status = bme.begin(0x76); 
  
  WiFiManager wifiManager;

  RS = digitalRead(rs);
  if (RS == LOW){
    wifiManager.resetSettings();
  }
  
  wifiManager.autoConnect("SmartStuff");

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

  if (!Firebase.RTDB.beginStream(&fbdo_ALL, STAT1)) {
    Serial.print ("ALL begin error ");
    Serial.println(fbdo_ALL.errorReason());
  }
  Firebase.RTDB.setFloat(&fbdo_ALL, STAT1, bme.readTemperature());
  Firebase.RTDB.setFloat(&fbdo_ALL, STAT2, bme.readHumidity());
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");
  
  server.begin();
  Serial.println("ALL DONE");
}

void loop(){
  Web();
  call();
  time();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
    Serial.println("Refresh token");
  }

}
void Web(){
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
            client.println("<h2>ELEMENT=" + ele + "</h2>");
            client.println("<h3>裝置名稱 = sMART sTUFF 溫溼度感測器</h3>");
            client.println("<h4>IP=" + IP + "</h4>");
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
void call(){
  if(Firebase.ready() && signupOK){
    if  (!Firebase.RTDB.readStream(&fbdo_ALL)){
       Serial.print("ALL read error：");
       Serial.println(fbdo_ALL.errorReason());
    }  
    if (fbdo_ALL.dataPath() == "/esp"){
      if (fbdo_ALL.stringData() == "1") {
        Firebase.RTDB.setString(&fbdo, CNSTAT, "2");
      } 
    }
    if (fbdo_ALL.dataPath() == "/Humidity"){
      if (fbdo_ALL.dataType() == "float") {
        Serial.print(fbdo_ALL.floatData());
        Serial.println(" % ");
      } 
    }
    if (fbdo_ALL.dataPath() == "/Temperature"){
      if (fbdo_ALL.dataType() == "float") {
        Serial.print(fbdo_ALL.floatData());
        Serial.print(" *C ");
      } 
    }
  }
}
void time() {
  timeClient.update();
  if (timeClient.getSeconds() == 0 || timeClient.getSeconds() == 15 || timeClient.getSeconds() == 30 || timeClient.getSeconds() == 45) {
    Firebase.RTDB.setFloat(&fbdo, STAT1, bme.readTemperature());
    Firebase.RTDB.setFloat(&fbdo, STAT2, bme.readHumidity());
  }
}