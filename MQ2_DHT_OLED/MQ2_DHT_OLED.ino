#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <MQ2.h>
#include <SPI.h>
#include <DHT.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 14 
#define DHTTYPE DHT11  

const int mq2_pin = A0;
const int buzz = D3;
MQ2 mq2(mq2_pin);
DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(115200);
  pinMode(buzz, OUTPUT);  digitalWrite(buzz, LOW);

  dht.begin();
  mq2.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop() {
  delay(1000);

  float t = dht.readTemperature();
  float s = mq2.readSmoke();
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");
  
  if(s >= 50){   //如果煙霧數值大於等於50 顯示偵測到煙霧及響起蜂鳴器
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("Smoke: ");
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.print("SMOKEING！！！");
    digitalWrite(buzz, HIGH);
  }
  else{        //如果煙霧數值小於50 顯示偵測到煙霧濃度數值
    display.setTextSize(1);
    display.setCursor(0, 35);
    display.print("Smoke: ");
    display.setTextSize(2);
    display.setCursor(0, 45);
    display.print(s);
    display.print(" RPM"); 
    digitalWrite(buzz, LOW);
  }
  
  
  display.display(); 
}