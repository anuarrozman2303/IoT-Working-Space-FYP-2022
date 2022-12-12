#define BLYNK_TEMPLATE_ID "TMPLVzbHHHYj"
#define BLYNK_DEVICE_NAME "IoT Working Space"
#define BLYNK_AUTH_TOKEN "zmFpMhwKFk1N0gjtJOte95wKcwC9B6WG"
#define BLYNK_PRINT Serial
#define DATABASE_URL "test-5a42b-default-rtdb.asia-southeast1.firebasedatabase.app"
#define DATABASE_SECRET "G8TMr0HZlm5buQZ8gJpdpQXr3d6lqmKH4KpPRLn6"

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
char AUTH[] = BLYNK_AUTH_TOKEN;

RTCZero rtc;   
const int GMT = +8;     // Time zone constant
char HOURS[3];
char MINUTES[3];
char SECONDS[3];
int DAY;
int MONTH;
String YEAR;

FirebaseData firebaseData;

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 0   //Pin DHT 11 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

#define SS_PIN  0  //Pin SDA RFID 
#define RST_PIN  0   //Pin RST RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define DOOR 0 //Pin Servo
Servo myservo; 

#define IRTX 0 //Pin IR Transmitter
#define LAMP1 0   //Pin Lamp 1
#define LAMP2 0    //Pin Lamp 2

void setupWIFI(){ //Done
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");
}

void setupNTP(){  //Done
  rtc.begin();          // Start Real Time Clock
  unsigned long epoch = 0;
  int numberOfTries = 0;
  int maxTries = 3;
    do {
    Serial.print("Attempt getting Server Time: "); Serial.print(numberOfTries+1);
    Serial.print("/"); Serial.println(maxTries);
    epoch = WiFi.getTime();
    numberOfTries++;
    delay(1000); //delay is for reading purpose on serial monitor
  } while ((epoch == 0) && (numberOfTries < maxTries));
}

void setupDISPLAY(){  //Done
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
}

void setupLED(){
  pinMode(LAMP1,OUTPUT); 
  pinMode(LAMP2,OUTPUT); 
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupWIFI();
  setupNTP();
  setupDISPLAY();
  setupMISC();
  setupLED();
}

/*-------------------------------------------LOOP------------------------------------------*/
void printTime(){
  sprintf(HOURS,"%02d", (rtc.getHours() + GMT));
  Serial.print(HOURS);
  sprintf(MINUTES,"%02d", rtc.getMinutes());
  Serial.print(":");
  Serial.print(MINUTES);
  sprintf(SECONDS,"%02d", rtc.getSeconds());
  Serial.print(":");
  Serial.println(SECONDS);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print(String(HOURS) + ":" + String(MINUTES));
}

void printDate(){ 
  //DAY = rtc.getDay();
  MONTH = rtc.getMonth();
  String M;
  switch (MONTH){
    case 1:
      M = " JAN";
    case 2:
      M = " FEB";
    case 3:
      M = " MAR";
    case 4:
      M = " APR";
    case 5:
      M = " MAY";
    case 6:
      M = " JUN";
    case 7:
      M = " JUL";
    case 8:
      M = " AUG";
    case 9:
      M = " SEP";
    case 10:
      M = " OCT";
    case 11:
      M = " NOV";
    case 12:
      M = " DEC";
    default:
      break;
  }
  YEAR = ("20" + String(rtc.getYear()));
  //Serial.print(DAY);
  //Serial.print("/");
  //Serial.print(M);
  //Serial.print("/");
  //Serial.println(YEAR);
  lcd.setCursor(0,0);
  lcd.print(rtc.getDay() + M + YEAR);
}

void loop() {
  // put your main code here, to run repeatedly:
  printTime();
  printDate();
  delay(1000);
  
}
