// https://www.grc.com/fingerprints.htm
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

String ele = "ledcontrol";
TaskHandle_t Task1;
TaskHandle_t Task2;

WebServer server(80);

String header;
const int LED_PIN = D9;
const int rs = D7;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

#define PIXEL_COUNT 16
#define PIXEL_PIN    6
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA"
#define USER_EMAIL "smart.stuff.18340@gmail.com"
#define USER_PASSWORD "Rayed18340"
FirebaseData fbdo, fbdo_ALL, fbdo_control;
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
String CONTROLLER = A + "/CONTROLLER";
String CONTROL = A + "/CONTROLBYCONTROLLER";
String MODE = A + "/MODE";
String CNSTAT = A + "/esp";
int R= "";
int G= "";
int B= "";
int base_color_set = 1;
int mode_set = 1

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);          digitalWrite(LED_PIN, HIGH);
  pinMode(rs, INPUT_PULLUP);         digitalWrite(rs, HIGH);
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
  if (Firebase.RTDB.getString(&fbdo_ALL, CONTROLLER)) {
    if (fbdo_ALL.stringData() != "0"){
      controller_MAC = "/" + fbdo_ALL.stringData();
    }
  }
  if (Firebase.RTDB.getString(&fbdo_ALL, COLOR)) {
    R = fbdo_ALL.stringData().substring(1, 4).toInt();
    G = fbdo_ALL.stringData().substring(4, 7).toInt();
    B = fbdo_ALL.stringData().substring(7, 10).toInt();
  }
  if (controller_MAC != ""){
    if (!Firebase.RTDB.beginStream(&fbdo_control, controller_MAC)) {
      Serial.print ("Control begin error ");
      Serial.println(fbdo_control.errorReason());
    }
  }
  Firebase.RTDB.setString(&fbdo_ALL, CONTROL, "0");
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");

  strip.begin(); 
  strip.setBrightness(255);
  strip.show(); 

  server.on("/info", handleINFO);
  server.onNotFound(handleNotFound);
  server.begin(); 
  Serial.println("ALL DONE");
  
}

void loop() {
  server.handleClient();

  if (controlby == "";){
    ReadStat();
  }
  if ((controller_MAC !="") && (controlby == "trig";)){
    controlbyMAC();
  }

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void handleINFO(void){
  server.send ( 200, "text/plain", "<h1>MAC=" + A + "</h1><h2>ELEMENT=" + ele + "</h2><h3>裝置名稱 = sMART sTUFF 智慧燈條</h3><h4>IP=" + IP + "</h4>" ); 
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
////////////////////////////////////////////////////////////////
      if (fbdo_ALL.dataPath() == "/COLOR"){
        Serial.println(fbdo_ALL.stringData());
        R = fbdo_ALL.stringData().substring(1, 4).toInt();
        G = fbdo_ALL.stringData().substring(4, 7).toInt();
        B = fbdo_ALL.stringData().substring(7, 10).toInt();
        switch(mode_set){
          case 1:  //恆亮
            colorWipe(strip.Color(R, G, B), 10);
            break;
          case 2:  //呼吸
            breath(strip.Color(R, G, B), 50);
            break;
          case 3:  //脈衝
            theaterChase(strip.Color(R, G, B), 50);
            break;
        }
      }
////////////////////////////////////////////////////////////////
      if (fbdo_ALL.dataPath() == "/MODE"){
        Serial.println(fbdo_ALL.stringData());
        mode_set = fbdo_ALL.stringData().toInt();
        switch(mode_set){
          case 1:  //恆亮
            colorWipe(strip.Color(R, G, B), 10);
            break;
          case 2:  //呼吸
            breath(strip.Color(R, G, B), 50);
            break;
          case 3:  //脈衝
            theaterChase(strip.Color(R, G, B), 50);
            break;
          case 4:  //彩虹
            rainbow(10);
            break;
          case 5:  //彩虹跑馬燈
            theaterChaseRainbow(50);
            break;
        }
      }
////////////////////////////////////////////////////////////////
      if (fbdo_ALL.dataPath() == "/CONTROLLER"){
        if (fbdo_ALL.stringData() != "0") {
          controller_MAC = "/" + fbdo.stringData();
        }
      }
////////////////////////////////////////////////////////////////
      if (fbdo_ALL.dataPath() == "/CONTROLBYCONTROLLER"){
        if (fbdo_ALL.stringData() == "1") {
          controlby = "trig";
          if (!Firebase.RTDB.beginStream(&fbdo_control, controller_MAC)) {
            Serial.print ("Control begin error ");
            Serial.println(fbdo.errorReason());
          }
        }
        else if (fbdo_ALL.stringData() == "0") {
          controlby = "";
        }
      }
    }
  }
}
void controlbyMAC(){
  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.readStream(&fbdo_control)) {
      Serial.print("CONTROL read error：");
      Serial.println(fbdo_control.errorReason());
    }
    if (fbdo_control.streamAvailable()) {
      if (fbdo_control.dataPath() == "/COLOR"){
        Serial.println(fbdo_control.stringData());
      }
      if (fbdo_control.dataPath() == "/MODE"){
        Serial.println(fbdo_control.stringData());
        mode_set = fbdo_control.stringData().toInt();
      }
    }
  }
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { 
    strip.setPixelColor(i, color);      
    strip.show();                     
  }
}

void breath(uint32_t color, int wait){
  for(int i=0; i<strip.numPixels(); i++) { 
    strip.setPixelColor(i, color);      
    strip.show();                 
  }
  int bri=0
  for(bri=0; bri<255; bri++){
    strip.setBrightness(bri);
    delay(wait);
  }
  for(bri=255; bri>0; bri--){
    strip.setBrightness(bri);
    delay(wait);
  }
  delay(1000);
}

void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {  // Repeat 10 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 3*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); 
    delay(wait); 
  }
}

void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;   
  for(int a=0; a<30; a++) {  
    for(int b=0; b<3; b++) { 
      strip.clear();  
      for(int c=b; c<strip.numPixels(); c += 3) {
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); 
        strip.setPixelColor(c, color); 
      }
      strip.show();          
      delay(wait);            
      firstPixelHue += 65536 / 90; 
    }
  }
}