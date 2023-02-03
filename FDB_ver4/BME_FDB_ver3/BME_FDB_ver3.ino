// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "bme";

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
unsigned long delayTime;

WiFiServer server(80);

String header;

const int rs = D4;
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
String T = "";
String H = "";
String STAT1 = "";
String STAT2 = "";

void setup() {
  Serial.begin(115200);
  
  pinMode(rs, INPUT);
  digitalWrite(rs, HIGH);
  bool status;
  status = bme.begin(0x76); 
  delayTime = 1000;
  
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
  
  STAT1 = hs + "/Temperature";
  STAT2 = hs + "/Humidity";
  
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
  Firebase.RTDB.setFloat(&fbdo, STAT1, bme.readTemperature());
  Firebase.RTDB.setFloat(&fbdo, STAT2, bme.readHumidity());

  server.begin();
}

void loop(){
  WiFiClient client = server.available();
  delay(delayTime);  


  
  if (Firebase.RTDB.getFloat(&fbdo, STAT1)) {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");
    T = fbdo.stringData() + " *C";
  }
  delay(10);
  if (Firebase.RTDB.getFloat(&fbdo, STAT2)) {
    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    H = fbdo.stringData() + " %";
  }
  delay(10);
  Firebase.RTDB.setFloat(&fbdo, STAT1, bme.readTemperature());
  Firebase.RTDB.setFloat(&fbdo, STAT2, bme.readHumidity());
  

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
            

            client.println("<body><h1>IP=" + A + "</h1>");
            client.println("<body><h2>ELEMENT=" + ele + "</h2>");
            client.println("<body><h3>裝置名稱 = sMART sTUFF 溫溼度感測器</h3>");
            client.println("<body><h4>目前溫度 : " + T + "</h4>");
            client.println("<body><h4>目前濕度 : " + H + "</h4>");

            
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