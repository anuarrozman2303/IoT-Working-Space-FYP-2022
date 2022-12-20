#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <IRremote.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <BlynkSimpleEsp32.h>
#include <FirebaseESP32.h>

/*****************************************************************/
const char* SSID   = "semanz";
const char* PASSWORD = "33632407";
const char* BLYNKAUTH = "zmFpMhwKFk1N0gjtJOte95wKcwC9B6WG";
/*****************************************************************/
const char* ntpServer = "my.pool.ntp.org";
const long  gmtOffset_sec = 28800;
const int   daylightOffset_sec = 28800;
unsigned long Started = 0;
const long interval = 1000;
char ntpHour[3];
char ntpMin[3];
char ntpSec[3];
/*****************************************************************/
//Blynk VPin
int vlamp1,vlamp2;
int IRLamp;
/*****************************************************************/
//Firebase

/*****************************************************************/
//LCD
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  
/*****************************************************************/
// Oled
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
/*****************************************************************/
//DHT
#define DHTPIN 13          //DHT PIN 13
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
/*****************************************************************/
//RFID
#define SS_PIN  5         //SDA PIN 6
#define RST_PIN 27         //RST PIN 7
MFRC522 mfrc522(SS_PIN, RST_PIN);
/*****************************************************************/
//MISC
#define buzzer 4 //pin buzzer
#define servoPin 2
#define PIN_SEND 15
#define lamp1 25   
#define lamp2 26
Servo myservo;
/*****************************************************************/

void setupWifi(){
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID,PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  
  printLocalTime();
}

void setupLCD(){
  lcd.begin();                                                            
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("IoT Working");
  lcd.setCursor(5,1);
  lcd.print("Space");
}

void setupOLED(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void setupLED(){
  pinMode(lamp1,OUTPUT);
  pinMode(lamp2,OUTPUT);
}

void setupMisc(){
  SPI.begin();          
  dht.begin();
  mfrc522.PCD_Init();        
  myservo.attach(servoPin);
  IrSender.begin(PIN_SEND);     
  Blynk.begin(BLYNKAUTH, SSID, PASSWORD);
}

//Blynk Virtual Pin
BLYNK_WRITE(V2) {
  vlamp1 = param.asInt();          //assigning incoming value from pin V2 to a variable
}

BLYNK_WRITE(V3) {
  vlamp2 = param.asInt(); 
}

BLYNK_WRITE(V4) {
  IRLamp = param.asInt();
  if (IRLamp == 1){
    myservo.detach();
    IrSender.sendNEC(0xFFE01F, 32);
    delay(100);
    IrSender.sendNEC(0xFFE01F, 32);
    delay(100);
    IrSender.sendNEC(0xFFE01F, 32); 
  }
  else if (IRLamp == 0){
    myservo.detach();
    IrSender.sendNEC(0xFF609F, 32);
    delay(100);
    IrSender.sendNEC(0xFF609F, 32);
    delay(100);
    IrSender.sendNEC(0xFF609F, 32); 
  } 
}

void setup(){
  Serial.begin(9600);
  setupWifi();          //Connect to WiFi
  setupLCD();        
  setupOLED();          
  setupLED();           
  setupMisc();
  Serial.println("HI");
}

void loop(){
  Serial.println("HI");
  printLocalTime();
  delay(1000);
  test();
}

void printLocalTime(){
  //unsigned long currentMillis = millis();
  //if (currentMillis - Started >= interval){
  //  Started = currentMillis;
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return;
    } 

    Serial.println("Date & Time: ");
    Serial.println(&timeinfo, "%B %d %Y");
    strftime(ntpHour,3, "%H", &timeinfo);
    Serial.print("Hours: ");
    Serial.println(ntpHour);
    strftime(ntpMin,3, "%M", &timeinfo);
    Serial.print("Mins: ");
    Serial.println(ntpMin);
    strftime(ntpSec,3, "%S", &timeinfo);
    Serial.print("Secs: ");
    Serial.println(ntpSec);
  //}
}

void test(){
  Serial.println(ntpHour);
  if (ntpHour == "20"){
    digitalWrite(lamp1, HIGH);
  }
}

