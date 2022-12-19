#define BLYNK_TEMPLATE_ID "TMPLVzbHHHYj"
#define BLYNK_DEVICE_NAME "IoT Working Space"
#define BLYNK_AUTH_TOKEN "zmFpMhwKFk1N0gjtJOte95wKcwC9B6WG"
#define BLYNK_PRINT Serial
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
char AUTH[] = BLYNK_AUTH_TOKEN;

RTCZero rtc;   
const int GMT = +8;     // Time zone constant
int Hours, Mins, Secs, Day, Month;
String Year;
int checkOutHours, checkOutMins;
unsigned long Started = 0;
const long interval = 1000;

FirebaseData firebaseData;

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 2   //Pin DHT 11 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

#define SS_PIN  7  //Pin SDA RFID 
#define RST_PIN 6   //Pin RST RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define DOOR 3 //Pin Servo
Servo myservo; 

#define ALARM 0 //Pin Buzzer
#define IRTX  1 //Pin IR Transmitter
#define LAMP1 4   //Pin Lamp 1
#define LAMP2 5    //Pin Lamp 2
#define rfidLED 0 //Pin Red LED RFID

#define ZMPT101B A0  //Pin AC Voltage Sensor 
int RawValue = 0; 
float testFrequency = 50; 
float windowLength = 125/testFrequency;
float Volts_TRMS;     // estimated actual voltage in Volts
float intercept = -5;  // to be adjusted based on calibration testin
float slope = 1.35; 
RunningStatistics inputStats; //This class collects the value so we can apply some functions

//Virtual Pin (BLYNK)
int vLAMP1; 
int vLAMP2;
int vIRFAN;

//Database Control
bool dbLamp1, dbLamp2;

/*--------------------------BLYNK Virtual Pin--------------------------*/
BLYNK_WRITE(V0) {
  vLAMP1 = param.asInt(); 
}

BLYNK_WRITE(V1) {
  vLAMP2 = param.asInt(); 
}

BLYNK_WRITE(V2) {
  vIRFAN = param.asInt(); 
}
/*----------------------------------------------------------------------*/

void setupWIFI(){ //Done
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");
}

void setupNTP(){
  rtc.begin();
  unsigned long epoch;    // Variable to represent epoch
  int numberOfTries = 0, maxTries = 6;    // Variable for number of tries to NTP service

  // Get epoch
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

void setupDISPLAY(){
  //LCD Setup
  lcd.begin();                                                            
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("IoT Working");
  lcd.setCursor(5,1);
  lcd.print("Space");

  //OLED Setup
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void setupMISC(){
  SPI.begin();          
  dht.begin();
  mfrc522.PCD_Init();        
  myservo.attach(DOOR);

  IrSender.begin(IRTX);     
  Blynk.begin(AUTH, SSID, PASSWORD);
  Firebase.begin(DATABASE_URL, DATABASE_SECRET, SSID, PASSWORD);
  Firebase.reconnectWiFi(true);
  inputStats.setWindowSecs( windowLength );
}

void setupLED(){
  pinMode(LAMP1,OUTPUT); 
  pinMode(LAMP2,OUTPUT); 
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
void rtcDateTime(){
  Hours = rtc.getHours() + GMT; 
  Mins = rtc.getMinutes(); 
  Secs = rtc.getSeconds();
  Day = rtc.getDay();
  Month = rtc.getMonth();
  Year = rtc.getYear();
  String M;
  switch (Month){
    case 1:
      M = " Jan";
    case 2:
      M = " Feb";
    case 3:
      M = " Mar";
    case 4:
      M = " Apr";
    case 5:
      M = " May";
    case 6:
      M = " Jun";
    case 7:
      M = " Jul";
    case 8:
      M = " Aug";
    case 9:
      M = " Sep";
    case 10:
      M = " Oct";
    case 11:
      M = " Nov";
    case 12:
      M = " Dec";
    default:
      break;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - Started >= interval) {
    Started = currentMillis;
    print2digits(rtc.getHours() + GMT);
    Serial.print(":");
    print2digits(rtc.getMinutes());
    Serial.print(":");
    print2digits(rtc.getSeconds());
    Serial.println("");
    Serial.println(Day + M + " 20" + Year);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(Day + M + " 20" + Year);
  }
  else{}
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

void controlvLAMP(){
  BLYNK_WRITE(vLAMP1);
  if (vLAMP1 == 1){
    digitalWrite(LAMP1,HIGH);
    Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP1", "ON");
    Serial.println("LAMP1:ON");
  }
    else{
      digitalWrite(LAMP1,LOW);
      Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP1", "OFF");
      Serial.println("LAMP1:OFF");
    }

  BLYNK_WRITE(vLAMP2);
  if (vLAMP2 == 1){
    digitalWrite(LAMP2,HIGH);
    Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP2", "ON");
    Serial.println("LAMP2:ON");
  }
    else){
      digitalWrite(LAMP2,LOW);
      Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP2", "OFF");
      Serial.println("LAMP2:OFF");
    }  

  BLYNK_WRITE(vIRLAMP);
  if (vIRFAN == 1){
    Firebase.setString(firebaseData, "/MEETING ROOM 1/FAN", "ON");
    myservo.detach();
    IrSender.sendNEC(0xFFE01F, 32);
    IrSender.sendNEC(0xFFE01F, 32);
    IrSender.sendNEC(0xFFE01F, 32);
    Serial.println("ON");    
  }
    else if (vIRFAN == 0){
      Firebase.setString(firebaseData, "/MEETING ROOM 1/FAN", "OFF");
      myservo.detach();
      IrSender.sendNEC(0xFF609F, 32);
      IrSender.sendNEC(0xFF609F, 32);
      IrSender.sendNEC(0xFF609F, 32);
      Serial.println("OFF");    
    } 
}

void offApp(){
  digitalWrite(LAMP1,LOW);
  Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP1", "OFF");
  digitalWrite(LAMP2,LOW);
  Firebase.setString(firebaseData, "/MEETING ROOM 1/LAMP2", "OFF");
  myservo.detach();
  IrSender.sendNEC(0xFF609F, 32);
  IrSender.sendNEC(0xFF609F, 32);
  IrSender.sendNEC(0xFF609F, 32); 
  Firebase.setString(firebaseData, "/MEETING ROOM 1/FAN", "OFF"); 
}
void rfidPermitted(){
    myservo.attach(DOOR);
    Firebase.setString(firebaseData, "/MAIN DOOR/RFID", "Permitted");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Welcome To IoT");
    lcd.setCursor(0,1);
    lcd.print("Working Space");
    myservo.write(180);
    digitalWrite(rfidLED, HIGH);
    tone(ALARM , 5000) ;
    delay(300) ;
  	noTone(ALARM) ; 
    delay(2000);
    digitalWrite(rfidLED, LOW);
    myservo.write(0);
    myservo.detach();
}

void rfidDenied(){
    Firebase.setString(firebaseData, "/MAIN DOOR/RFID", "Denied");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sorry Wrong Key");
    digitalWrite(rfidLED, HIGH);
    tone(ALARM,5000);
    delay(300);
    digitalWrite(rfidLED, LOW);
  	noTone(ALARM);
    delay(300);
    digitalWrite(rfidLED, HIGH);
    tone(ALARM,5000);
    delay(300);
    digitalWrite(rfidLED, LOW);
    noTone(ALARM);
}

void rfidDOOR(){
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  if (content.substring(1) == "D2 57 D5 1A") //change here the UID of the card/cards that you want to give access
  {
    rfidPermitted();  
  }
 
  else   
  {
    rfidDenied();
  }
}

void dhtOLED(){
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  Blynk.virtualWrite(V3, t);
  Blynk.virtualWrite(V4, h);
  Firebase.setFloat(firebaseData, "/Meeting ROOM 1/TEMPERATURE", t);
  Firebase.setFloat(firebaseData, "/Meeting ROOM 1/HUMIDITY", h);

  display.clearDisplay();
  
  // display temperature
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

void getCheckOut(){   //Get Check Out Time From Database
  if (Firebase.getInt(firebaseData, "/CHECKOUT/HOURS")){
    if (firebaseData.dataType() == "int"){
      checkOutHours = (firebaseData.intData());
    }
  }
  if (Firebase.getInt(firebaseData, "/CHECKOUT/MINUTES")){
    if (firebaseData.dataType() == "int"){
      checkOutMins = (firebaseData.intData());
    }
  }
}

void controlAPP(){
  if (Hours != checkOutHours && Mins != checkOutMins){
    controlvLAMP();
  }
    else{
       offApp();
    }
}

void voltSensor(){
    RawValue = analogRead(ZMPT101B);  // read the analog in value:
    inputStats.input(RawValue);       // log to Stats function
        
    unsigned long currentMillis = millis();
    if (currentMillis - Started >= interval) {
      Started = currentMillis;
      
      Volts_TRMS = inputStats.sigma()* slope + intercept;
      Serial.print("AC Voltage: ");
      Serial.print("\t");
      Serial.println(Volts_TRMS);
  }
}

void loop() {
  rtcDateTime();
  Blynk.run();
  getCheckOut();
  controlAPP();
  dhtOLED();
  rfidDOOR();
}