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
int rs = 1;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

#define LED_PIN    10
#define LED_COUNT 120
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
int MinBrightness = 0;     
int MaxBrightness = 255;  
int fadeInWait = 15;     
int fadeOutWait = 15; 

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA"
#define USER_EMAIL "smart.stuff.18340@gmail.com"
#define USER_PASSWORD "Rayed18340"
FirebaseData fbdo, fbdo_ALL, fbdo_control, fbdo_STRIP;
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
int R= 255;
int G= 0;
int B= 0;
int mode_set = 1;

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
  if (!Firebase.RTDB.beginStream(&fbdo_STRIP, A)) {
    Serial.print ("STRIP begin error ");
    Serial.println(fbdo_STRIP.errorReason());
  }
  if (Firebase.RTDB.getString(&fbdo_ALL, CONTROLLER)) {
    if (fbdo_ALL.stringData() != "0"){
      controller_MAC = fbdo_ALL.stringData();
    }
    if (!Firebase.RTDB.beginStream(&fbdo_control, controller_MAC)) {
      Serial.print ("control begin error ");
      Serial.println(fbdo_control.errorReason());
    }
  }
  if (Firebase.RTDB.getString(&fbdo_ALL, COLOR)) {
    R = fbdo_ALL.stringData().substring(0, 3).toInt();
    G = fbdo_ALL.stringData().substring(3, 6).toInt();
    B = fbdo_ALL.stringData().substring(6, 9).toInt();
  }
  else if(!Firebase.RTDB.getString(&fbdo_ALL, COLOR)) {
    Firebase.RTDB.setString(&fbdo_ALL, COLOR, "255000000");
  }
  Firebase.RTDB.setString(&fbdo_ALL, CONTROL, "0");
  Firebase.RTDB.setString(&fbdo_ALL, MODE, "1");
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
  ReadStat();

  if (controlby == "trig"){
    controlbyMAC();
  }
  if (controlby != "trig"){
    controlbySTRIP();
  }
  color_change();
  
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
    if(controller_MAC != ""){
      Firebase.RTDB.setString(&fbdo_control, controller_MAC + "/" + A, "2");
    }
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
      Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
      Serial.println(fbdo_ALL.errorReason());
    }
    if (fbdo_ALL.streamAvailable()) {
      if (fbdo_ALL.dataPath() == "/esp"){
        if (fbdo_ALL.stringData() == "1") {
          Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "2");
        }
      }
////////////////////////////////////////////////////////////////
      if (fbdo_ALL.dataPath() == "/CONTROLLER"){
        if (fbdo_ALL.stringData() != "0") {
          controller_MAC =  fbdo_ALL.stringData();
          Serial.print("Now Connect To Controller : ");
          Serial.println(controller_MAC);
          if (!Firebase.RTDB.beginStream(&fbdo_control, controller_MAC)) {
            Serial.print ("control begin error ");
            Serial.println(fbdo_control.errorReason());
          }
        }
        else{
          controller_MAC = "";
          Serial.print("Controller Disconnect");
        }
      }
////////////////////////////////////////////////////////////////
      if (fbdo_ALL.dataPath() == "/CONTROLBYCONTROLLER"){
        if (fbdo_ALL.stringData() == "1") {
          if(controller_MAC != ""){
              Firebase.RTDB.setString(&fbdo_control, controller_MAC + "/" + A, "1");
              controlby = "trig";
              if(Firebase.RTDB.getString(&fbdo_control, controller_MAC+"/COLOR")){
                R = fbdo_control.stringData().substring(0, 3).toInt();
                G = fbdo_control.stringData().substring(3, 6).toInt();
                B = fbdo_control.stringData().substring(6, 9).toInt();
              }
              if(Firebase.RTDB.getString(&fbdo_control, controller_MAC+"/MODE")){
                mode_set = fbdo_control.stringData().toInt();
              }
              Serial.println("NOW CONTROL BY CONTROLLER");
          }
          else{
            Firebase.RTDB.setString(&fbdo_ALL, CONTROL, "0");
          }
        }
        else if (fbdo_ALL.stringData() == "0") {
          Firebase.RTDB.setString(&fbdo_control, controller_MAC + "/" + A, "0");
          Firebase.RTDB.setString(&fbdo_STRIP, CONTROL, "2");
          controlby = "";
          if(Firebase.RTDB.getString(&fbdo_STRIP, COLOR)){
            R = fbdo_STRIP.stringData().substring(0, 3).toInt();
            G = fbdo_STRIP.stringData().substring(3, 6).toInt();
            B = fbdo_STRIP.stringData().substring(6, 9).toInt();
          }
          if(Firebase.RTDB.getString(&fbdo_STRIP, MODE)){
            mode_set = fbdo_STRIP.stringData().toInt();
          }
          Serial.println("NOW CONTROL BY STRIP");
        }
      }
    }
  }
}
void controlbySTRIP(){
  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.readStream(&fbdo_STRIP)) {
      Firebase.RTDB.setString(&fbdo_STRIP, CONTROL, "0");
      Serial.print("STRIP read error：");
      Serial.println(fbdo_STRIP.errorReason());
    }
    if (fbdo_STRIP.streamAvailable()) {
      if (fbdo_STRIP.dataPath() == "/COLOR"){
        Serial.println(fbdo_STRIP.stringData());
        R = fbdo_STRIP.stringData().substring(0, 3).toInt();
        G = fbdo_STRIP.stringData().substring(3, 6).toInt();
        B = fbdo_STRIP.stringData().substring(6, 9).toInt();
      }
////////////////////////////////////////////////////////////////
      if (fbdo_STRIP.dataPath() == "/MODE"){
        Serial.println(fbdo_STRIP.stringData());
        mode_set = fbdo_STRIP.stringData().toInt();
      }
    }
  }
}

void controlbyMAC(){
  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.readStream(&fbdo_control)) {
      Firebase.RTDB.setString(&fbdo_control, controller_MAC + "/" + A, "1");
      Serial.print("CONTROL read error：");
      Serial.println(fbdo_control.errorReason());
    }
    if (fbdo_control.streamAvailable()) {
      if (fbdo_control.dataPath() == "/COLOR"){
        Serial.println(fbdo_control.stringData());
        R = fbdo_control.stringData().substring(0, 3).toInt();
        G = fbdo_control.stringData().substring(3, 6).toInt();
        B = fbdo_control.stringData().substring(6, 9).toInt();
      }
      if (fbdo_control.dataPath() == "/MODE"){
        Serial.println(fbdo_control.stringData());
        mode_set = fbdo_control.stringData().toInt();
      }
    }
  }
}

void color_change(){
  switch(mode_set){
    case 1:  //單色恆亮
      colorWipe(strip.Color(R, G, B), 5);
      break;
    case 2:  //單色呼吸
      rgbBreathe(strip.Color(R, G, B), 50);
      break;
    case 3:  //單色跑馬燈
      theaterChase(strip.Color(R, G, B), 50);
      break;
    case 4:  //彩虹
      rainbow(10);
      break;
    case 5:  //彩虹呼吸燈
      rainbowBreathe(500); 
      break;
    case 6:
      close_led(strip.Color(R, G, B), 5);
      break;
  }  
}

void close_led(uint32_t color, int wait){
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setBrightness(0);   
    delay(wait);            
  }
  strip.show(); 
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setBrightness(255); 
    strip.setPixelColor(i, color);            
  }
  strip.show();  
}

void theaterChase(uint32_t color, int wait) {
  strip.setBrightness(255); 
  for(int a=0; a<5; a++) { 
    for(int b=0; b<3; b++) { 
      strip.clear();       
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color);
      }
      strip.show(); 
      delay(wait);  
    }
  }
}

void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 1*65536; firstPixelHue += 256) {
    strip.setBrightness(255); 
    strip.rainbow(firstPixelHue);
    strip.show();
    delay(wait); 
  }
}

void rgbBreathe(uint32_t c, uint8_t y) {
  for (int j = 0; j < 1; j++) {
    for (uint8_t b = MinBrightness; b < MaxBrightness; b++) {
      strip.setBrightness(b * 255 / 255);
      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
      }
      strip.show();
      delay(fadeInWait);
    }
    strip.setBrightness(MaxBrightness * 255 / 255);
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(y);
    }
    for (uint8_t b = MaxBrightness; b > MinBrightness; b--) {
      strip.setBrightness(b * 255 / 255);
      for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
      }
      strip.show();
      delay(fadeOutWait);
    }
  }
  delay(10);
}

void rainbowBreathe(uint8_t y) {
  for (int j = 0; j < 1; j++) {
    for (uint8_t b = MinBrightness; b < MaxBrightness; b++) {
      strip.setBrightness(b * 255 / 255);
      for (uint8_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(i * 256 / strip.numPixels()));
      }
      strip.show();
      delay(fadeInWait);
    }
    strip.setBrightness(MaxBrightness * 255 / 255);
    for (uint8_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(i * 256 / strip.numPixels()));
      strip.show();
      delay(y);
    }
    for (uint8_t b = MaxBrightness; b > MinBrightness; b--) {
      strip.setBrightness(b * 255 / 255);
      for (uint8_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(i * 256 / strip.numPixels()));
      }
      strip.show();
      delay(fadeOutWait);
    }
  }
}



uint32_t Wheel(byte WheelPos) {
  WheelPos = 140 - WheelPos;       //the value here means - for 255 the strip will starts with red, 127-red will be in the middle, 0 - strip ends with red.
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}