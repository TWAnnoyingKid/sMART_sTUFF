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

String ele = "1stat";

WiFiServer server(80);

String header;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

String output1State = "on";

const int output1 = D1;
const int rs = D4;
int RS; 

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA" 
FirebaseData fbdo, fbdo_D1;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = "";
String STAT1 = "";

void setup() {
  Serial.begin(115200);
  
  pinMode(output1, OUTPUT);    digitalWrite(output1, LOW);
  pinMode(rs, INPUT_PULLUP);   digitalWrite(rs, HIGH);
  
  WiFiManager wifiManager;
  RS = digitalRead(rs);
  if (RS == LOW){
    wifiManager.resetSettings();
  }
  
  wifiManager.autoConnect("sMART sTUFF");
  A = WiFi.macAddress();
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(28800);
  
  STAT1 = A + "/D1";
  
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
  
  Firebase.RTDB.setString(&fbdo_D1, STAT1, "1");
  Firebase.RTDB.setString(&fbdo, A+"/OT/D1", "0");
  Firebase.RTDB.setString(&fbdo, A+"/CT/D1", "0");
  if  (!Firebase.RTDB.beginStream(&fbdo_D1, STAT1)){
    Serial.printf("D1 begin error", fbdo_D1.errorReason().c_str());
  }
  server.begin();
  Serial.println("done");  
}

void loop(){
  timeClient.update();
  time();

  if(Firebase.ready() && signupOK){
    if  (!Firebase.RTDB.readStream(&fbdo_D1)){
      Serial.printf("D1 read error", fbdo_D1.errorReason().c_str());
    }
    if (fbdo_D1.streamAvailable()) {
      if (fbdo_D1.stringData() == "1") {
        Serial.println("D1 on");
        output1State = "on";
        digitalWrite(output1, LOW);
      }  
      else if (fbdo_D1.stringData() == "0") {
        Serial.println("D1 off");
        output1State = "off";
        digitalWrite(output1, HIGH);
      }
    }  
  }
  WEB();
}
void time(){
  String a = "\\\"";
  String nowTime = a + timeClient.getHours() + ":" + timeClient.getMinutes() + a ;
  Serial.println(timeClient.getSeconds());  
  delay(1000);
  if(timeClient.getSeconds()<5){
    if (Firebase.RTDB.getString(&fbdo, A+"/OT/D1")) {
      if (fbdo.stringData() == nowTime) {
        Firebase.RTDB.setString(&fbdo_D1, STAT1, "1");    
      }
    }
    if (Firebase.RTDB.getString(&fbdo, A+"/CT/D1")) {
      if (fbdo.stringData() == nowTime) {
       Firebase.RTDB.setString(&fbdo_D1, STAT1, "0");
      }
    }
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
            
            if (header.indexOf("GET /1/on") >= 0) {
              Firebase.RTDB.setString(&fbdo_D1, STAT1, "1");
              output1State = "on";
              digitalWrite(output1, LOW);
            } 
            else if (header.indexOf("GET /1/off") >= 0) {
              Firebase.RTDB.setString(&fbdo_D1, STAT1, "0");
              output1State = "off";
              digitalWrite(output1, HIGH);
            }
            

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
            client.println("<body><h3>裝置名稱 = sMART sTUFF 智慧插座</h3>");
            

            client.println("<p>D1 - State " + output1State + "</p>");
            if (output1State=="off") {
              client.println("<p><a href=\"/1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/1/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            
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