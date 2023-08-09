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
TaskHandle_t Task1;
TaskHandle_t Task2;
WebServer server(80);

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
int close_gate = 16;
int open_gate = 4;
int D0 = 21;
int rs = 2;
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
String open_time = "";
String gate_stat="";

String A = WiFi.macAddress();
String a = "\\\"";
String IP = "";
String STAT1 = A + "/D1";
String CNSTAT = A + "/esp";
String LED_RESET = "NON";
String LED_SETTING = "NON";
String LED_OPEN = "NON";
String uid = "";
String OnNum = "1";
String CloseNum = "0";

void setup() {
  Serial.begin(115200);
  input_password.reserve(12); 
  pinMode(open_gate, OUTPUT);  digitalWrite(open_gate, LOW);
  pinMode(close_gate, OUTPUT); digitalWrite(close_gate, LOW);
  pinMode(LED_PIN, OUTPUT);    digitalWrite(LED_PIN, HIGH);
  pinMode(rs, INPUT_PULLUP);   digitalWrite(rs, HIGH);
  pinMode(D0, INPUT); 
  digitalWrite(open_gate, LOW);
  digitalWrite(close_gate, HIGH);
  delay(1500);
  digitalWrite(open_gate, LOW);
  digitalWrite(close_gate, LOW);
  gate_stat="close";
  xTaskCreatePinnedToCore(
                    Task1code, 
                    "Task1",    
                    20000,     
                    NULL,      
                    0,      
                    &Task1, 
                    0);  
  LED_SETTING = "ON";
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
    LED_RESET = "ON";
  }
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
  server.on("/info", handleINFO);
  server.onNotFound(handleNotFound);
  server.begin(); 
  Serial.println("ALL DONE");
  LED_SETTING = "OFF";
  LED_RESET = "OFF";
}

void loop() {
  door_open();
  sro4();
  rfid_scan();
  ReadStat();
  time();

  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "1");
    Serial.println("Refresh token");
  }
}
void sro4(){
  Serial.println(digitalRead(D0));
  if (digitalRead(D0) == 0){
    if(gate_stat == "close" ){
      digitalWrite(open_gate, HIGH);
      digitalWrite(close_gate, LOW);
      delay(5*1000);
      digitalWrite(open_gate, LOW);
      digitalWrite(close_gate, HIGH);
      // gate_stat="open";
      // Firebase.RTDB.setString(&fbdo_ALL, STAT1, 0);
      open_time_saving();
    }
    else if(gate_stat == "open" ){
      digitalWrite(open_gate, LOW);
      digitalWrite(close_gate, HIGH);
      gate_stat="close";
      Firebase.RTDB.setString(&fbdo_ALL, STAT1, 0);
      open_time_saving();
    }
  }
  delay(100);
}

void rfid_scan(){
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.print(F("Card UID:"));
    dump_byte_array(rfid.uid.uidByte, rfid.uid.size); 
    rfid.PICC_HaltA(); 
  }
}

void dump_byte_array(byte *readCard, byte readCardSize) {
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  delay(50);
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
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
    LED_OPEN = "CLOSE";
    gate_stat="close";
    digitalWrite(open_gate, LOW);
    digitalWrite(close_gate, HIGH);
    open_time_saving();
  }
  else if (Firebase.RTDB.getString(&fbdo_ALL, A+"/UID/"+uid)){
    Firebase.RTDB.setString(&fbdo_ALL, A + "/TEMP_UID", "1");
    if (fbdo_ALL.stringData() == "1"){
      Serial.println(" ");
      Serial.println("Valid CARD => unlock the door");
      LED_OPEN = "OPEN";
      gate_stat="open";
      digitalWrite(open_gate, HIGH);
      digitalWrite(close_gate, LOW);
      open_time_saving();
    }
    else if (fbdo_ALL.stringData() == "0"){
      Serial.println(" ");
      Serial.println("Invalid CARD => Try again");
      LED_OPEN = "CLOSE";
      gate_stat="close";
      digitalWrite(open_gate, LOW);
      digitalWrite(close_gate, HIGH);
      open_time_saving();
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
      if (input_password == password_1 || input_password == password_temp) {
        Serial.println(" ");
        Serial.println("Valid Password => unlock the door");
        LED_OPEN = "OPEN";
        gate_stat="open";
        digitalWrite(open_gate, HIGH);
        digitalWrite(close_gate, LOW);
        open_time_saving();

        lcd.setCursor(0, 0);
        lcd.print("CORRECT!");
        lcd.setCursor(0, 1);
        lcd.print("DOOR UNLOCKED!");
      } 
      else {
        Serial.println(" ");
        Serial.println("Invalid Password => Try again");
        LED_OPEN = "CLOSE";
        gate_stat="close";
        digitalWrite(open_gate, LOW);
        digitalWrite(close_gate, HIGH);
        open_time_saving();
        
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
void handleINFO(void){
  server.send ( 200, "text/plain", "<h1>MAC=" + A + "</h1><h2>ELEMENT=" + ele + "</h2><h3>裝置名稱 = sMART sTUFF 智慧門鎖</h3><h4>IP=" + IP + "</h4>" ); 
}
void handleNotFound(){                                 
  server.send(404, "text/plain", "404: Not found");   
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
          digitalWrite(open_gate, LOW);
          digitalWrite(close_gate, HIGH);
          open_time_saving();
          LED_OPEN = "CLOSE";
          gate_stat="close";
          Serial.println("Door Locked");
        } else if (fbdo_ALL.stringData() == OnNum) {
          digitalWrite(open_gate, HIGH);
          digitalWrite(close_gate, LOW);
          open_time_saving();
          LED_OPEN = "OPEN";
          gate_stat="open";
          Serial.println("Door Opened");
        }
      }
      if (fbdo_ALL.stringData().substring(2, 4) == "D1"){
        if(fbdo_ALL.stringData().substring(7, 8).toInt() == CloseNum){
          digitalWrite(open_gate, LOW);
          digitalWrite(close_gate, HIGH);
          open_time_saving();
          LED_OPEN = "CLOSE";
          gate_stat="close";
          Serial.println("Door Locked");
        }else if(fbdo_ALL.stringData().substring(7, 8).toInt() == OnNum){
          digitalWrite(open_gate, HIGH);
          digitalWrite(close_gate, LOW);
          open_time_saving();
          LED_OPEN = "OPEN";
          gate_stat="open";
          Serial.println("Door Opened");
        }
      }
      if (fbdo_ALL.dataPath() == "/esp"){
        if (fbdo_ALL.stringData() == "1") {
          Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "2");
        } 
      }
      if (fbdo_ALL.stringData().substring(2, 5) == "esp"){
        Firebase.RTDB.setString(&fbdo_ALL, CNSTAT, "2");
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
            password_1.replace(a,"");
            Serial.println("password_1 now is " + password_1);
          }
          else{
            password_1 = "";
            Serial.println("Password fail");
          }
        } 
      }
      if (fbdo_Password.stringData().substring(2, 5) == "PW1"){
        String PW = fbdo_Password.stringData().substring(10, 22)
        PW.replace("\"","");
        PW.replace("\\","");
        PW.replace("}","");
        if(PW != "0"){
          password_1 = PW;
          Serial.println("password_1 now is " + password_1);
        }else{
          password_1 = "";
          Serial.println("Password fail");
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
      if (fbdo_Password.stringData().substring(2, 8) == "PWTemp"){
        String TPW = fbdo_Password.stringData().substring(13, 25)
        TPW.replace("\"","");
        TPW.replace("\\","");
        TPW.replace("}","");
        if (TPW != "0"){
          password_temp = TPW;
          temp_time_saving();
          Serial.println("password_temp now is " + password_temp);
        }else{
          temp_time = "";
          Serial.println("Temp Password fail");
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
void open_time_saving(){
  timeClient.update();
  open_time = timeClient.getSeconds();
  Serial.println("Gate Open Saving Time：" + open_time);
}

void time(){
  timeClient.update();
  if(open_time != ""){
    int opt = timeClient.getSeconds() - open_time.toInt();
    if(opt > 0){
      if(opt == 5){
        if(digitalRead(open_gate) == HIGH){
          digitalWrite(open_gate, LOW);
        }
        if(digitalRead(close_gate) == HIGH){
          digitalWrite(close_gate, LOW);
        }
        Serial.println("Gate Open Saving Time FAILD");
        open_time = "";
      }
    }
    else if(opt < 0){
      if(opt == -55){
        if(digitalRead(open_gate) == HIGH){
          digitalWrite(open_gate, LOW);
        }
        if(digitalRead(close_gate) == HIGH){
          digitalWrite(close_gate, LOW);
        }
        Serial.println("Gate Open Saving Time FAILD");
        open_time = "";
      }
    }
  }
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
void Task1code( void * pvParameters ){
  for(;;){
    server.handleClient();
    // sro4();
    door_open();
    while (LED_SETTING == "ON"){
      digitalWrite(LED_PIN, HIGH);
      delay(1000);
      digitalWrite(LED_PIN, LOW);
      delay(1000);
    }
    if (LED_OPEN == "CLOSE"){
      digitalWrite(LED_PIN, HIGH);
      delay(2000);
      digitalWrite(LED_PIN, LOW);
      LED_OPEN = "NON";
    }
    if (LED_OPEN == "OPEN"){
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      LED_OPEN = "NON";
    }
    if (LED_RESET == "ON"){
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
    }
    if (LED_OPEN == "NON" || LED_SETTING == "OFF" || LED_RESET == "OFF"){
      digitalWrite(LED_PIN, LOW);
    }
  } 
}