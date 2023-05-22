#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
#include <SPI.h>
#include <MFRC522.h>
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

#define ROW_NUM     4 
#define COLUMN_NUM  4  
#define SS_PIN  5  
#define RST_PIN 17 

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

#define DATABASE_URL "https://esp8266-ai2-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAF4OdtYUAomk_4WnvE5MXb_nphlQ33UyA"
#define USER_EMAIL "smart.stuff.18340@gmail.com"
#define USER_PASSWORD "Rayed18340"
FirebaseData fbdo, fbdo_ALL, fbdo_Password;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
int output1 = 16;
int rs = 4;
int LED_PIN = 0;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pin_rows[ROW_NUM] = {32, 33, 25, 26};  
byte pin_column[COLUMN_NUM] = {27, 14, 12, 13}; 
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27,16,2);  // SDA to GPIO21 || SCL to GPIO22

String password_1 = ""; 
String password_temp = ""; 
String temp_time = "";
String input_password;

String A = WiFi.macAddress();
String a = "\\\"";
String IP = "";
String STAT1 = A + "/D1";
String CNSTAT = A + "/esp";
String uid = "";
String OnNum = "1";
String CloseNum = "0";

void setup() {
  Serial.begin(115200);
  input_password.reserve(12); 
  pinMode(output1, OUTPUT);   digitalWrite(output1, HIGH);
  pinMode(LED_PIN, OUTPUT);   digitalWrite(LED_PIN, HIGH);
  pinMode(rs, INPUT_PULLUP);  digitalWrite(rs, HIGH);
  Wire.begin (21, 22);
  lcd.init(); 
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("sETTING...");
  lcd.setCursor(0,1);
  lcd.print("WIFI");

  WiFiManager wifiManager;
  // wifiManager.resetSettings();
  if (digitalRead(rs) == LOW) {
    wifiManager.resetSettings();
  }
  digitalWrite(LED_PIN, LOW);
  wifiManager.autoConnect("sMART sTUFF");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("sETTING...");
  lcd.setCursor(0,1);
  lcd.print("OTHER sTUFFs");

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
  Firebase.RTDB.setString(&fbdo_ALL, A + "/TEMP_UID", "0");
  Firebase.RTDB.setString(&fbdo_Password, A + "/Password/PWTemp", "0");
  if (Firebase.RTDB.getString(&fbdo_Password, A + "/Password/PW1")){
    password_1 = fbdo_Password.stringData();
    Serial.println("password_1 now is " + password_1);
  }
  SPI.begin(); 
  rfid.PCD_Init(); 
  server.begin();
  Serial.println("ALL DONE");
  
}

void loop() {
  door_open();
  rfid_scan();
  WEB();
  ReadStat();
  time();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void rfid_scan(){
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.print(F("Card UID:"));
    dump_byte_array(rfid.uid.uidByte, rfid.uid.size); 
    rfid.PICC_HaltA(); 
  }
}

void dump_byte_array(byte *readCard, byte readCardSize) {
  uid = "";
  for (byte i = 0; i < readCardSize; i++) {
    Serial.print(readCard[i] < 0x10 ? "0" : "");
    uid += String(readCard[i],HEX);
  }
  Serial.println(uid);
  if (!Firebase.RTDB.getString(&fbdo_ALL, A+"/UID/"+uid)){
    Firebase.RTDB.setString(&fbdo_ALL, A + "/TEMP_UID", uid);
    Serial.println(" ");
    Serial.println("Invalid CARD => Try again");
    digitalWrite(output1, HIGH);
  }
  else if (Firebase.RTDB.getString(&fbdo_ALL, A+"/UID/"+uid)){
    Firebase.RTDB.setString(&fbdo_ALL, A + "/TEMP_UID", "1");
    if (fbdo_ALL.stringData() == "1"){
      Serial.println(" ");
      Serial.println("Valid CARD => unlock the door");
      digitalWrite(output1, LOW);
    }
    else if (fbdo_ALL.stringData() == "0"){
      Serial.println(" ");
      Serial.println("Invalid CARD => Try again");
      digitalWrite(output1, HIGH);
    }
  }
  Serial.println("DONE");
}
void door_open(){
  char key = keypad.getKey();

  if (key) {
    Serial.print(key);

    if (key == '*') {
      input_password = "";
      Serial.println(" ");
      Serial.println("Reset Input");
      lcd.clear();
    } 
    else if (key == '#') {
      lcd.clear();
      if (a + input_password + a == password_1 || input_password == password_temp) {
        Serial.println(" ");
        Serial.println("Valid Password => unlock the door");
        digitalWrite(output1, LOW);
        lcd.setCursor(0, 0);
        lcd.print("CORRECT!");
        lcd.setCursor(0, 1);
        lcd.print("DOOR UNLOCKED!");
      } 
      else {
        Serial.println(" ");
        Serial.println("Invalid Password => Try again");
        digitalWrite(output1, HIGH);
        lcd.setCursor(0, 0);
        lcd.print("INCORRECT!");
        lcd.setCursor(0, 1);
        lcd.print("ACCESS DENIED!");
      }
      input_password = ""; 
    } 
    else {
      if (input_password.length() == 0) {
        lcd.clear();
      }

      input_password += key; 
      lcd.setCursor(input_password.length(), 0); 
      lcd.print('*');                 
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