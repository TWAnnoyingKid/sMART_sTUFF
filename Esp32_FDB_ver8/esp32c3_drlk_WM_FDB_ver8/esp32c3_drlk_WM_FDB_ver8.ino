#include <Keypad.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
String ele = "drlk";
WiFiServer server(80);

String header;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.stdtime.gov.tw");

#define ROW_NUM     4  // four rows
#define COLUMN_NUM  4  // four columns

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA"
#define USER_EMAIL "smart.stuff.18340@gmail.com"
#define USER_PASSWORD "Rayed18340"
FirebaseData fbdo, fbdo_ALL, fbdo_Password, fbdo_OT;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
int output1 = 12;
int rs = 14;
int LED_PIN = 27;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// byte pin_rows[ROW_NUM] = {1, 2, 3, 4};  
// byte pin_column[COLUMN_NUM] = {5, 6, 7, 8}; 
byte pin_rows[ROW_NUM] = {19, 18, 5, 17};  
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};  
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

String password_1 = ""; 
String password_temp = ""; 
String temp_time = "";
String input_password;

String A = WiFi.macAddress();
String a = "\\\"";
String IP = "";
String STAT1 = A + "/D1";
String CNSTAT = A + "/esp";

String OnNum = "1";
String CloseNum = "0";

void setup() {
  Serial.begin(115200);
  input_password.reserve(17); 
  pinMode(output1, OUTPUT);   digitalWrite(output1, HIGH);
  pinMode(LED_PIN, OUTPUT);   digitalWrite(LED_PIN, HIGH);
  pinMode(rs, INPUT_PULLUP);  digitalWrite(rs, HIGH);

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

  if (!Firebase.RTDB.beginStream(&fbdo_ALL, A)) {
    Serial.print ("ALL begin error ");
    Serial.println(fbdo_ALL.errorReason());
  }
  if (!Firebase.RTDB.beginStream(&fbdo_Password, A + "/Password")) {
    Serial.print ("Password begin error ");
    Serial.println(fbdo_ALL.errorReason());
  }  
  
  Firebase.RTDB.setString(&fbdo_ALL, STAT1, CloseNum);
  Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "0");
  Firebase.RTDB.setString(&fbdo_Password, A + "/Password/PWTemp", "0");
  if (Firebase.RTDB.getString(&fbdo_Password, A + "/Password/PW1")){
    password_1 = fbdo_Password.stringData();
    Serial.println("password_1 now is " + password_1);
  }
  Serial.println("ALL DONE");
  server.begin();


}

void loop() {
  door_open();
  WEB();
  ReadStat();
  time();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void door_open(){
  char key = keypad.getKey();

  if (key) {
    Serial.print(key);

    if (key == '*') {
      input_password = ""; 
      Serial.println(" ");
      Serial.println("Reset Input");
    } 
    else if (key == '#') {
      if (a + input_password + a == password_1 || input_password == password_temp) {
        Serial.println(" ");
        Serial.println("Valid Password => unlock the door");
      } 
      else {
        Serial.println(" ");
        Serial.println("Invalid Password => Try again");
      }
      input_password = ""; 
    } 
    else {
      input_password += key; 
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
    while (client.connected() && currentTime - previousTime <= timeoutTime) { 
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
            client.println("<h3>裝置名稱 = sMART sTUFF 智慧門鎖</h3>");
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
void ReadStat(){
  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.readStream(&fbdo_ALL)) {
      Serial.print("D1 read error：");
      Serial.println(fbdo_ALL.errorReason());
    }
    if (fbdo_ALL.streamAvailable()) {
      if (fbdo_ALL.dataPath() == "/D1"){
        if (fbdo_ALL.stringData() == CloseNum) {
          digitalWrite(output1, HIGH);
          Serial.println("Door Locked");
        } else if (fbdo_ALL.stringData() == OnNum) {
          digitalWrite(output1, LOW);
          Serial.println("Door Opened");
        }
      }
      if (fbdo_ALL.dataPath() == "/esp"){
        if(fbdo_ALL.dataType() == "string"){
          if (fbdo_ALL.stringData() == "1") {
            Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "2");
          }
        }
      }
    }

    if (!Firebase.RTDB.readStream(&fbdo_Password)) {
      Serial.print("Password read error：");
      Serial.println(fbdo_Password.errorReason());
    }
    if (fbdo_Password.streamAvailable()) {
      if (fbdo_Password.dataPath() == "/PW1"){
        if(fbdo_Password.dataType() == "string"){
          if (fbdo_Password.stringData() != "0"){
            password_1 = fbdo_Password.stringData();
            Serial.println("password_1 now is " + password_1);
          }
          else{
            password_1 = "";
            Serial.println("Password fail");
          }
        } 
      }
      if (fbdo_Password.dataPath() == "/PWTemp"){
        if(fbdo_Password.dataType() == "string"){
          if (fbdo_Password.stringData() != "0"){
            password_temp = fbdo_Password.stringData();
            temp_time_saving();
            Serial.println("password_temp now is " + password_temp);
          }
          else {
            temp_time = "";
            Serial.println("Temp Password fail");
          }
        } 
      }
    }
  }
}
void temp_time_saving(){
  timeClient.update();
  temp_time = timeClient.getMinutes();
  Serial.println("Temp Password Saving Time：" + temp_time);
}
void time(){
  timeClient.update();
  if (timeClient.getSeconds() > 0 & timeClient.getSeconds() < 3) {
    if(temp_time != ""){
      int g = timeClient.getMinutes() - temp_time.toInt();
      if(g > 0){
        if(g == 3){ 
          Serial.println("Temp Password fail");
          Firebase.RTDB.setString(&fbdo_ALL, A + "/Password/PWTemp", "0");
          temp_time = "";
          password_temp = "";
        }
      }
      else if(g < 0){ 
        if(g == (-57)){
          Serial.println("Temp Password fail");
          Firebase.RTDB.setString(&fbdo_ALL, A + "/Password/PWTemp", "0");
          temp_time = "";
          password_temp = "";
        }
      }
    }
  }
}