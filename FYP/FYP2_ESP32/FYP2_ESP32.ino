#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLVzbHHHYj"
#define BLYNK_DEVICE_NAME "IoT Working Space"
#define BLYNK_AUTH_TOKEN "zmFpMhwKFk1N0gjtJOte95wKcwC9B6WG"
#define FIREBASE_HOST     "test-5a42b-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH     "EinObk758cqiUwi05wRJNDtBmiweE4FXv68dlBWh"
#define APP_KEY           "47f05c0b-8f2c-4b74-a5d2-c00aec445c57"      
#define APP_SECRET        "4b9fa934-ee9b-4fd0-8a7c-c4a3b9807804-e696b366-91f3-47a6-b2f9-c8f3489cf562"   

#include "ZMPT101B.h"
#include "EmonLib.h" 
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <FirebaseESP32.h>
#include <BlynkSimpleEsp32.h>
#include <addons/TokenHelper.h>   // Provide the token generation process info.
#include <addons/RTDBHelper.h>    // Provide the RTDB payload printing info and other helper functions.

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#define IRAPP             "639094ceb8a7fefbd65770cc"    
#define LAMP1             "638f7c72b8a7fefbd6568b49"   
#define LAMP2             "6390948e333d12dd2a0bd4bd"  

EnergyMonitor emon1;  

ZMPT101B voltageSensor(35);

float Vrms, Irms, aPower, kWh;

unsigned long interval = 1000;
unsigned long previousMillis1 = 0;

char SSID[] = "nuar";
char PASSWORD[] = "anuar123";
char AUTH[] = BLYNK_AUTH_TOKEN;

FirebaseData fbdo;

void setupWIFI() {
  WiFi.begin(SSID , PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");
  Blynk.begin(AUTH, SSID, PASSWORD);
}

void setupFB() {
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void setupSinricPro() {
  SinricProSwitch& mySwitch1 = SinricPro[IRAPP];
  mySwitch1.onPowerState(onPowerState1);

  SinricProSwitch& mySwitch2 = SinricPro[LAMP1];
  mySwitch2.onPowerState(onPowerState2);

  SinricProSwitch& mySwitch3 = SinricPro[LAMP2];
  mySwitch3.onPowerState(onPowerState3);

  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setupEnergy(){
  //Turn off power source during start up.
  voltageSensor.calibrate();
  emon1.current(34, 30); 
}

void setup(){
  Serial.begin(9600);
  setupWIFI();
  setupFB();
  setupSinricPro();
  setupEnergy();
}

void energyMonitor(){
  Irms = emon1.calcIrms(5588) - 1;  //Irms
  Vrms = voltageSensor.getVoltageAC(); //Vrms

  if (Irms < 0){
    Irms = 0;
    Serial.println(String("Irms = ") + Irms + " A");
    Blynk.virtualWrite(V6, Irms);     
  }
  else {
    Serial.println(String("Irms = ") + Irms + " A"); 
    Blynk.virtualWrite(V6, Irms); 
  }

  if (Vrms < 200){
    Vrms = 0;
    Serial.println(String("Vrms = ") + Vrms + " V");
    Blynk.virtualWrite(V5, Vrms); 
  }
  else{
    Serial.println(String("Vrms = ") + Vrms + " V");
    Blynk.virtualWrite(V5, Vrms); 
  }

  aPower = Vrms * Irms; //P = VI
  Serial.println(String("Apparent Power = ") + aPower + " W");
  Blynk.virtualWrite(V7, aPower); 

  kWh = aPower / 1000;  //kWh = W/1000hours
  Serial.println(String("kWh = ") + kWh + " kWh");
  Blynk.virtualWrite(V8, kWh); 
}

void loop(){
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - previousMillis1) >= interval) {
    energyMonitor();
    previousMillis1 = millis();
  }
  SinricPro.handle();
  Blynk.run();
}

bool onPowerState1(const String &deviceId, bool &state) {
  if(state){
    Firebase.setBool(fbdo, "/CONTROL/IR APP", true); 
    Serial.println("IRAPP Firebase: ON");
  }    
  else if (!state){
    Firebase.setBool(fbdo, "/CONTROL/IR APP", false);
    Serial.println("IRAPP Firebase: OFF");
  }
  return true;
}

bool onPowerState2(const String &deviceId, bool &state) {
  if(state){
    Firebase.setBool(fbdo, "/CONTROL/LAMP1", true); 
    Serial.println("LAMP 1 Firebase: ON");
  }    
  else if (!state){
    Firebase.setBool(fbdo, "/CONTROL/LAMP1", false);
    Serial.println("LAMP 1 Firebase: OFF");
  }
  return true;
}

bool onPowerState3(const String &deviceId, bool &state) {
  if(state){
    Firebase.setBool(fbdo, "/CONTROL/LAMP2", true); 
    Serial.println("LAMP 2 Firebase: ON");
  }    
  else if (!state){
    Firebase.setBool(fbdo, "/CONTROL/LAMP2", false);
    Serial.println("LAMP 2 Firebase: OFF");
  }
  return true;
}