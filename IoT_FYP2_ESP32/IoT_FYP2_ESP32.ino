#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>   // Provide the token generation process info.
#include <addons/RTDBHelper.h>    // Provide the RTDB payload printing info and other helper functions.


#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#define WIFI_SSID         "semanz"    
#define WIFI_PASS         "33632407"
#define FIREBASE_HOST     "test-5a42b-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH     "EinObk758cqiUwi05wRJNDtBmiweE4FXv68dlBWh"
#define APP_KEY           "47f05c0b-8f2c-4b74-a5d2-c00aec445c57"      
#define APP_SECRET        "4b9fa934-ee9b-4fd0-8a7c-c4a3b9807804-e696b366-91f3-47a6-b2f9-c8f3489cf562"   
#define IRAPP             "639094ceb8a7fefbd65770cc"    
#define LAMP1             "638f7c72b8a7fefbd6568b49"   
#define LAMP2             "6390948e333d12dd2a0bd4bd"  

FirebaseData fbdo;

void setupWIFI() {
  WiFi.begin(WIFI_SSID , WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");
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

  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup() {
  Serial.begin(9600);
  setupWIFI();
  setupFB();
  setupSinricPro();
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

void loop() {
  SinricPro.handle();
}
