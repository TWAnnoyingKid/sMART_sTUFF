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

WiFiServer server(80);

String header;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const int water_pin = D1;
const int rs = D4;
int RS; 

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA" 
#define USER_EMAIL "smart.stuff.18340@gmail.com"
#define USER_PASSWORD "Rayed18340"
FirebaseData fbdo, fbdo_D1, fbdo_D4;
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
  wifiManager.autoConnect("SmartStuff");
  IP = WiFi.localIP().toString();

  timeClient.begin();
  timeClient.setTimeOffset(28800);
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
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
  
  if  (!Firebase.RTDB.beginStream(&fbdo_D1, WATER)){
    Serial.printf("WATER begin error", fbdo_D1.errorReason().c_str());
  }
  Firebase.RTDB.setString(&fbdo, WATER, "0");
  Firebase.RTDB.setString(&fbdo, A + "/SetMoi/High", "0");
  Firebase.RTDB.setString(&fbdo, A + "/SetMoi/Low", "0");

  server.begin();
  Serial.println("done"); 
}

void loop(){
  time();
  set_moi();
  WATERING();
  WEB();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_D4, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void time(){
  timeClient.update();
  String a = "\\\"";
  String nowTime = a + timeClient.getHours() + ":" + timeClient.getMinutes() + a ;
  
  if(timeClient.getSeconds()<5){
    if (Firebase.RTDB.getString(&fbdo, A+"/SetMoi/TIME")) {
      if (fbdo.stringData() == nowTime) {
        Serial.println("WATERING...");  
        Firebase.RTDB.setString(&fbdo_D1, WATER, "1");  
        delay(6*1000);  
        Serial.println("STOP WATER");
        Firebase.RTDB.setString(&fbdo_D1, WATER, "0");  
      }
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
      Firebase.RTDB.setString(&fbdo_D1, WATER, "1");  
    }
    else if(moipercent > HM){
      Firebase.RTDB.setString(&fbdo_D1, WATER, "0"); 
    }
  }
}
void WATERING(){
  if(Firebase.ready() && signupOK){
    if  (!Firebase.RTDB.readStream(&fbdo_D1)){
      Serial.printf("WATER read error", fbdo_D1.errorReason().c_str());
    }
    if (fbdo_D1.streamAvailable()) {
      if (fbdo_D1.stringData() == "0") {
        Serial.println("STOP WATER");
        digitalWrite(water_pin, HIGH);
      }  
      else if (fbdo_D1.stringData() == "1") {
        Serial.println("WATERING..."); 
        digitalWrite(water_pin, LOW);
      }
    }
    if (!Firebase.RTDB.readStream(&fbdo_D4)) {
      Serial.print("cn read error：");
      Serial.println(fbdo_D4.errorReason());
    }
    if (fbdo_D4.streamAvailable()) {
      if (fbdo_D4.stringData() == "1") {
        Firebase.RTDB.setString(&fbdo_D4, CNSTAT, "2");
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

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset='utf-8' name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style>");
            client.println("</head>");
            

            client.println("<body><h1>MAC=" + A + "</h1>");
            client.println("<h2>ELEMENT=" + ele + "</h2>");
            client.println("<h3>裝置名稱 = sMART sTUFF 智慧盆栽</h3>");
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