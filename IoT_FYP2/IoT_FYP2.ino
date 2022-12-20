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

char SSID[] = "semanz";
char PASSWORD[] = "33632407";

RTCZero rtc;
const int GMT = +8;  // Time zone constant
int Hours, Mins, Secs, Day, Month;
String Year;
int checkOutHours, checkOutMins;
unsigned long timeStarted = 0;
const long intTime = 1000;

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

#define DOOR 6  //Pin Servo
Servo myservo;

#define ALARM 4    //Pin Buzzer
#define IRTX 1     //Pin IR Transmitter
#define LAMP1 20    //Pin Lamp 1
#define LAMP2 21    //Pin Lamp 2
#define rfidLED 0  //Pin Red LED RFID

//Database Control
bool dbLamp1, dbLamp2, dbIRApp;

void setupWIFI() {  //Done
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");
}

void setupNTP() {
  rtc.begin();
  unsigned long epoch;                  // Variable to represent epoch
  int numberOfTries = 0, maxTries = 6;  // Variable for number of tries to NTP service

  // Get epoch
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }

  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    while (1)
      ;
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
  myservo.write(0);

  IrSender.begin(IRTX);
  Firebase.begin(DATABASE_URL, DATABASE_SECRET, SSID, PASSWORD);
  Firebase.reconnectWiFi(true);
}

void setupLED() {
  pinMode(LAMP1, OUTPUT);
  pinMode(LAMP2, OUTPUT);
  pinMode(rfidLED, OUTPUT);
  pinMode(ALARM, OUTPUT);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupWIFI();
  setupDISPLAY();
  setupMISC();
  setupNTP();
  setupLED();
}

/*-------------------------------------------LOOP------------------------------------------*/
void printTime(){
  print2digits(rtc.getHours() + GMT);
  Serial.print(":");
  
  print2digits(rtc.getMinutes());
  Serial.print(":");
  
  print2digits(rtc.getSeconds());
  Serial.println();
}

void printDate(){
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
  if (currentMillis - timeStarted >= intTime) {
    timeStarted = currentMillis;
    printDate();
    printTime();
  }
  else{}
}

void controlvLAMP() {
  if (dbLamp1 == 1) {
    digitalWrite(LAMP1, HIGH);
    Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP1", "ON");
  } else {
    digitalWrite(LAMP1, LOW);
    Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP1", "OFF");
  }

  if (dbLamp2 == 1) {
    digitalWrite(LAMP2, HIGH);
    Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP2", "ON");
  } else {
    digitalWrite(LAMP2, LOW);
    Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP2", "OFF");
  }

  if (dbIRApp == 1) {
    Firebase.setString(firebaseData, "/MEETING ROOM 1/IR APP", "ON");
    myservo.detach();
    IrSender.sendNEC(0xFFE01F, 32);
    IrSender.sendNEC(0xFFE01F, 32);
    IrSender.sendNEC(0xFFE01F, 32);
  } else {
    Firebase.setString(firebaseData, "/MEETING ROOM 1/IR APP", "OFF");
    myservo.detach();
    IrSender.sendNEC(0xFF609F, 32);
    IrSender.sendNEC(0xFF609F, 32);
    IrSender.sendNEC(0xFF609F, 32);
  }
}

void offApp() {
  digitalWrite(LAMP1, LOW);
  Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP1", "OFF");
  digitalWrite(LAMP2, LOW);
  Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP2", "OFF");
  myservo.detach();
  IrSender.sendNEC(0xFF609F, 32);
  IrSender.sendNEC(0xFF609F, 32);
  IrSender.sendNEC(0xFF609F, 32);
  Firebase.setString(firebaseData, "/MEETING ROOM 1/IR APP", "OFF");
}

void rfidPermitted() {
  Serial.println("Permitted");
  myservo.attach(DOOR);
  Firebase.setString(firebaseData, "/IoT Working Space/DOOR", "Permitted");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome To IoT");
  lcd.setCursor(0, 1);
  lcd.print("Working Space");
  myservo.write(180);
  digitalWrite(rfidLED, HIGH);
  tone(ALARM, 5000);
  delay(300);
  noTone(ALARM);
  digitalWrite(rfidLED, LOW);
  delay(1000);
  myservo.write(0);
  myservo.detach();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IoT Work Space");
}

void rfidDenied() {
  Serial.println("Denied");
  Firebase.setString(firebaseData, "/IoT Working Space/DOOR", "Denied");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sorry Wrong Key");
  digitalWrite(rfidLED, HIGH);
  tone(ALARM, 5000);
  delay(300);
  digitalWrite(rfidLED, LOW);
  noTone(ALARM);
  delay(300);
  digitalWrite(rfidLED, HIGH);
  tone(ALARM, 5000);
  delay(300);
  digitalWrite(rfidLED, LOW);
  noTone(ALARM);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IoT Work Space");
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
    rfidPermitted();
  }

  else {
    rfidDenied();
  }
}

void dhtOLED() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  Blynk.virtualWrite(V3, t);
  Blynk.virtualWrite(V4, h);
  Firebase.setFloat(firebaseData, "/MEETING ROOM 1/TEMPERATURE", t);
  Firebase.setFloat(firebaseData, "/MEETING ROOM 1/HUMIDITY", h);

  display.clearDisplay();

  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(h);
  display.print(" %");

  display.display();
}

void getCheckOut() {  //Get Check Out Time From Database
  if (Firebase.getInt(firebaseData, "/CHECKOUT/HOURS")) {
    if (firebaseData.dataType() == "int") {
      checkOutHours = (firebaseData.intData());
    }
  }
  if (Firebase.getInt(firebaseData, "/CHECKOUT/MINUTES")) {
    if (firebaseData.dataType() == "int") {
      checkOutMins = (firebaseData.intData());
    }
  }
}

void getDatabaseControl() { 
  if (Firebase.getBool(firebaseData, "/CONTROL/LAMP1")) {
    if (firebaseData.dataType() == "boolean") {
      dbLamp1 = (firebaseData.boolData());
    }
  }
  if (Firebase.getBool(firebaseData, "/CONTROL/LAMP2")) {
    if (firebaseData.dataType() == "boolean") {
      dbLamp2 = (firebaseData.boolData());
    }
  }
  if (Firebase.getBool(firebaseData, "/CONTROL/IR APP")) {
    if (firebaseData.dataType() == "boolean") {
      dbIRApp = (firebaseData.boolData());
    }
  }
}

void controlAPP() {
  Hours = rtc.getHours() + GMT;
  Mins = rtc.getMinutes();
  if (Hours != checkOutHours && Mins != checkOutMins) {
    controlvLAMP();
  } else {}
}

void loop() {
  TIME();
  lcdDate();
  rfidDOOR();
  dhtOLED();
  getCheckOut();
  getDatabaseControl();
  controlAPP();
}