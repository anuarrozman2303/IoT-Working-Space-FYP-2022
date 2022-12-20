#define DATABASE_URL "test-5a42b-default-rtdb.asia-southeast1.firebasedatabase.app"
#define DATABASE_SECRET "EinObk758cqiUwi05wRJNDtBmiweE4FXv68dlBWh"

#include <WiFi101.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <MFRC522.h>
#include <Servo.h>
#include <IRremote.h>
#include <BlynkSimpleMKR1000.h>
#include <Firebase_Arduino_WiFi101.h>
#include <Filters.h>

char SSID[] = "semanz";
char PASSWORD[] = "33632407";
int status = WL_IDLE_STATUS;

RTCZero rtc;
const int GMT = +8;  // Time zone constant
int Hours, Mins;
int checkOutHours, checkOutMins;
unsigned long Started = 0;
const long interval = 1000;

FirebaseData firebaseData;

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 2  //Pin DHT 11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SS_PIN 7   //Pin SDA RFID
#define RST_PIN 5  //Pin RST RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define DOOR 3  //Pin Servo
Servo myservo;

#define ALARM 6    //Pin Buzzer
#define IRTX 1     //Pin IR Transmitter
#define LAMP1 20    //Pin Lamp 1
#define LAMP2 21    //Pin Lamp 2
#define rfidLED 0  //Pin Red LED RFID

//Database Control
bool dbLamp1, dbLamp2, dbIRApp;

void setupWIFIRTC() {
  Serial.begin(115200);

  if (WiFi.status() == WL_NO_SHIELD) {
    // Wait until WiFi ready
    Serial.println("WiFi adapter not ready");
    while (true);
  }

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SSID);
    status = WiFi.begin(SSID, PASSWORD);

    delay(5000);
  }

  rtc.begin();
  
  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;

  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }

  while ((epoch == 0) && (numberOfTries < maxTries));

    if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    while (1);
    }

    else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);
    Serial.println();
    }
}

void setupDISPLAY() {
  //LCD Setup
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IoT Work Space");

  //OLED Setup
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void setupMISC() {
  SPI.begin(); // init SPI bus
  mfrc522.PCD_Init(); // init MFRC522
  Serial.println("RFID IS READY");
  dht.begin();
  myservo.attach(DOOR);

  IrSender.begin(IRTX);
  Firebase.begin(DATABASE_URL, DATABASE_SECRET, SSID, PASSWORD);
  Firebase.reconnectWiFi(true);
}

void setupLED() {
  pinMode(LAMP1, OUTPUT);
  pinMode(LAMP2, OUTPUT);
  pinMode(rfidLED, OUTPUT);
}

void setup(){
  setupWIFIRTC();
  setupMISC();
  setupDISPLAY();
}

void loop(){
  rfidDOOR();
  TIME();
  lcdDate();
}

void rfidDOOR() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  if (content.substring(1) == "D2 57 D5 1A")  //change here the UID of the card/cards that you want to give access
  {
    Serial.println("OK");
    digitalWrite(rfidLED,HIGH);
    delay(100);
    digitalWrite(rfidLED,LOW);
    //rfidPermitted();
  }

  else {
    Serial.println("TAK OK");
    //rfidDenied();
  }
}

void printTime()
{
  print2digits(rtc.getHours() + GMT);
  Serial.print(":");
  
  print2digits(rtc.getMinutes());
  Serial.print(":");
  
  print2digits(rtc.getSeconds());
  Serial.println();
}

void printDate()
{
  Serial.print(rtc.getDay());
  Serial.print("/");
  
  Serial.print(rtc.getMonth());
  Serial.print("/");
  
  Serial.print(rtc.getYear());
  Serial.print(" ");
}

String print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

void lcdDate(){
  lcd.setCursor(0,1);
  lcd.print(String(rtc.getDay()) + "/" + String(rtc.getMonth()) + "/" + "20" + String(rtc.getYear()));
}

// Set Time to Run Every 1 Sec
void TIME() {
  unsigned long currentMillis = millis();
  if (currentMillis - Started >= interval) {
    Started = currentMillis;
    printDate();
    printTime();
  }
  else{}
}