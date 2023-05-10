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

WiFiServer server(80);

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
FirebaseData fbdo, fbdo_D1, fbdo_D2, fbdo_D3, fbdo_D4, fbdo_D1_TC, fbdo_D2_TC, fbdo_D3_TC, fbdo_D1_TO, fbdo_D2_TO, fbdo_D3_TO ;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

String A = WiFi.macAddress();
String a = "\\\"";
String D1_TC = "";
String D2_TC = "";
String D3_TC = "";
String D1_TO = "";
String D2_TO = "";
String D3_TO = "";

String IP = "";
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

  if (!Firebase.RTDB.beginStream(&fbdo_D1, STAT1)) {
    Serial.print ("D1 begin error ");
    Serial.println(fbdo_D1.errorReason());
  }
  if (!Firebase.RTDB.beginStream(&fbdo_D2, STAT2)) {
    Serial.print ("D2 begin error ");
    Serial.println(fbdo_D2.errorReason());
  }
  if (!Firebase.RTDB.beginStream(&fbdo_D3, STAT3)) {
    Serial.print ("D3 begin error ");
    Serial.println(fbdo_D3.errorReason());
  }

  if (!Firebase.RTDB.beginStream(&fbdo_D1_TC, A + "/CT/D1")) {
    Serial.print ("D1_TC begin error ");
    Serial.println(fbdo_D1.errorReason());
  }
  if (!Firebase.RTDB.beginStream(&fbdo_D2_TC, A + "/CT/D2")) {
    Serial.print ("D2_TC begin error ");
    Serial.println(fbdo_D2.errorReason());
  }
  if (!Firebase.RTDB.beginStream(&fbdo_D3_TC, A + "/CT/D3")) {
    Serial.print ("D3_TC begin error ");
    Serial.println(fbdo_D3.errorReason());
  }

  if (!Firebase.RTDB.beginStream(&fbdo_D1_TO, A + "/OT/D1")) {
    Serial.print ("D1_TO begin error ");
    Serial.println(fbdo_D1.errorReason());
  }
  if (!Firebase.RTDB.beginStream(&fbdo_D2_TO, A + "/OT/D2")) {
    Serial.print ("D2_TO begin error ");
    Serial.println(fbdo_D2.errorReason());
  }
  if (!Firebase.RTDB.beginStream(&fbdo_D3_TO, A + "/OT/D3")) {
    Serial.print ("D3_TO begin error ");
    Serial.println(fbdo_D3.errorReason());
  }

  if (!Firebase.RTDB.beginStream(&fbdo_D4, CNSTAT)) {
    Serial.print ("CN begin error ");
    Serial.println(fbdo_D4.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo_D1_TO, A + "/OT/D1")) {
    D1_TO = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_D2_TO, A + "/OT/D2")) {
    D2_TO = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_D3_TO, A + "/OT/D3")) {
    D3_TO = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_D1_TC, A + "/CT/D1")) {
    D1_TC = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_D2_TC, A + "/CT/D2")) {
    D2_TC = fbdo.stringData();
  }
  if (Firebase.RTDB.getString(&fbdo_D3_TC, A + "/CT/D3")) {
    D3_TC = fbdo.stringData();
  }

  Firebase.RTDB.setString(&fbdo_D1, STAT1, OnNum);
  Firebase.RTDB.setString(&fbdo_D2, STAT2, OnNum);
  Firebase.RTDB.setString(&fbdo_D3, STAT3, OnNum);
  Firebase.RTDB.setString(&fbdo_D4, CNSTAT, "0");
  server.begin();
  Serial.println("ALL DONE");
  
  
}

void loop() {
  ReadStat();
  time();
  change_stat();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_D4, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void time() {
  timeClient.update();
  String nowTime = a + timeClient.getHours() + ":" + timeClient.getMinutes() + a;
  if (timeClient.getSeconds() < 7 & timeClient.getSeconds() > 0) {
    if (D1_TO == nowTime) {
      D1_TO_TRI = "TURE"
    } 
    if (D2_TO == nowTime) {
      D2_TO_TRI = "TURE"
    } 
    if (D3_TO == nowTime) {
      D3_TO_TRI = "TURE"
    } 
    if (D1_TC == nowTime) {
      D1_TC_TRI = "TURE"
    } 
    if (D2_TC == nowTime) {
      D2_TC_TRI = "TURE"
    } 
    if (D3_TC == nowTime) {
      D3_TC_TRI = "TURE"
    }
  }
}
void WEB() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {
        // header = client.readStringUntil('\r');
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
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}</style>");
            client.println("</head><body>");
            client.println("<h1>MAC=" + A + "</h1>");
            client.println("<h2>ELEMENT=" + ele + "</h2>");
            client.println("<h3>裝置名稱 = sMART sTUFF 三插座智慧延長線</h3>");
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
  }
}

void ReadStat() {

  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.readStream(&fbdo_D1)) {
      Serial.print("D1 read error：");
      Serial.println(fbdo_D1.errorReason());
    }
    if (fbdo_D1.streamAvailable()) {
      if (fbdo_D1.stringData() == CloseNum) {
        digitalWrite(output1, HIGH);
        Serial.println("D1 OFF");
      } else if (fbdo_D1.stringData() == OnNum) {
        digitalWrite(output1, LOW);
        Serial.println("D1 ON");
      }
    }
    if (!Firebase.RTDB.readStream(&fbdo_D2)) {
      Serial.print("D2 read error：");
      Serial.println(fbdo_D2.errorReason());
    }
    if (fbdo_D2.streamAvailable()) {
      if (fbdo_D2.stringData() == CloseNum) {
        digitalWrite(output2, HIGH);
        Serial.println("D2 OFF");
      } else if (fbdo_D2.stringData() == OnNum) {
        digitalWrite(output2, LOW);
        Serial.println("D2 ON");
      }
    }
    if (!Firebase.RTDB.readStream(&fbdo_D3)) {
      Serial.print("D3 read error：");
      Serial.println(fbdo_D3.errorReason());
    }
    if (fbdo_D3.streamAvailable()) {
      if (fbdo_D3.stringData() == CloseNum) {
        digitalWrite(output3, HIGH);
        Serial.println("D3 OFF");
      } else if (fbdo_D3.stringData() == OnNum) {
        digitalWrite(output3, LOW);
        Serial.println("D3 ON");
      }
    }
////////////////////////////////////////////////////////////
    if (!Firebase.RTDB.readStream(&fbdo_D1_TO)) {
      Serial.print("D1_TO read error：");
      Serial.println(fbdo_D1_TO.errorReason());
    }
    if (fbdo_D1_TO.streamAvailable()) {
      D1_TO = fbdo_D1_TO.stringData()
    }
    if (!Firebase.RTDB.readStream(&fbdo_D2_TO)) {
      Serial.print("D2_TO read error：");
      Serial.println(fbdo_D2_TO.errorReason());
    }
    if (fbdo_D2_TO.streamAvailable()) {
      D2_TO = fbdo_D2_TO.stringData()
    }
    if (!Firebase.RTDB.readStream(&fbdo_D3_TO)) {
      Serial.print("D3_TO read error：");
      Serial.println(fbdo_D3_TO.errorReason());
    }
    if (fbdo_D3_TO.streamAvailable()) {
      D3_TO = fbdo_D3_TO.stringData()
    }
////////////////////////////////////////////////////////////
    if (!Firebase.RTDB.readStream(&fbdo_D1_TC)) {
      Serial.print("D1_TC read error：");
      Serial.println(fbdo_D1_TC.errorReason());
    }
    if (fbdo_D1_TC.streamAvailable()) {
      D1_TC = fbdo_D1_TC.stringData()
    }
    if (!Firebase.RTDB.readStream(&fbdo_D2_TC)) {
      Serial.print("D2_TC read error：");
      Serial.println(fbdo_D2_TC.errorReason());
    }
    if (fbdo_D2_TC.streamAvailable()) {
      D2_TC = fbdo_D2_TC.stringData()
    }
    if (!Firebase.RTDB.readStream(&fbdo_D3_TC)) {
      Serial.print("D3_TC read error：");
      Serial.println(fbdo_D3_TC.errorReason());
    }
    if (fbdo_D3_TC.streamAvailable()) {
      D3_TC = fbdo_D3_TC.stringData()
    }
////////////////////////////////////////////////////////////
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
void change_stat(){
    if(d1button != "non"){
      if (digitalRead(output1) == LOW) {
        Firebase.RTDB.setString(&fbdo_D1, STAT1, OnNum);
        d1button = "non";
      } else if (digitalRead(output1) == HIGH) {
        Firebase.RTDB.setString(&fbdo_D1, STAT1, CloseNum);
        d1button = "non";
      }
    }
    delay(100);
    if(d2button != "non"){
      if (digitalRead(output2) == LOW) {
        Firebase.RTDB.setString(&fbdo_D2, STAT2, OnNum);
        d2button = "non";
      } else if (digitalRead(output2) == HIGH) {
        Firebase.RTDB.setString(&fbdo_D2, STAT2, CloseNum);
        d2button = "non";
      }
    }
    delay(100);
    if(d3button != "non"){
      if (digitalRead(output3) == LOW) {
        Firebase.RTDB.setString(&fbdo_D3, STAT3, OnNum);
        d3button = "non";
      } else if (digitalRead(output3) == HIGH) {
        Firebase.RTDB.setString(&fbdo_D3, STAT3, CloseNum);
        d3button = "non";
      }
    }
    delay(100);
    if(D1_TO_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_D1, STAT1, OnNum);
      D1_TO_TRI = "non"
    }
    if(D2_TO_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_D2, STAT2, OnNum);
      D2_TO_TRI = "non"
    }
    if(D3_TO_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_D3, STAT3, OnNum);
      D3_TO_TRI = "non"
    }
    delay(100);
    if(D1_TC_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_D1, STAT1, CloseNum);
      D1_TC_TRI = "non"
    }
    if(D2_TC_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_D2, STAT2, CloseNum);
      D2_TC_TRI = "non"
    }
    if(D3_TC_TRI != "non"){
      Firebase.RTDB.setString(&fbdo_D3, STAT3, CloseNum);
      D3_TC_TRI = "non"
    }
    delay(100);
}
void Task1code( void * pvParameters ){
  for(;;){
    WEB();
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