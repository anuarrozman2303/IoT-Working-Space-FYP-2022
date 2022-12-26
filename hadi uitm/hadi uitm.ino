
 
#include "EmonLib.h"   //https://github.com/openenergymonitor/EmonLib
#include <WiFi.h>
#include <WiFiClient.h>

EnergyMonitor emon;
#define vCalibration 83.0
#define currCalibration 0.169

float kWh = 0;
unsigned long lastmillis = millis();


void myTimerEvent() {
    emon.calcVI(20, 2000);
    Serial.print("Vrms: ");
    Serial.print(emon.Vrms, 2);
    Serial.println("V");

    Serial.print("\tIrms: ");
    Serial.print(emon.Irms, 4);
    Serial.println("A");

    Serial.print("\tPower: ");
    Serial.print(emon.apparentPower, 4);
    Serial.println("W");

    Serial.print("\tkWh: ");
    kWh = kWh + emon.apparentPower*(millis()-lastmillis)/3600000000.0;
    Serial.print(kWh, 4);
    Serial.println("kWh");
    lastmillis = millis();

}
 
void setup() {
  Serial.begin(9600);
  emon.voltage(35, vCalibration, 1.7); // Voltage: input pin, calibration, phase_shift
  emon.current(34, currCalibration); // Current: input pin, calibration.
  //timer.setInterval(5000L, myTimerEvent);
}
 
void loop() {
  myTimerEvent();
  
}