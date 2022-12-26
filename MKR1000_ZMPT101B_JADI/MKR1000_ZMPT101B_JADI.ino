#include "ZMPT101B.h"
#include "EmonLib.h" 

EnergyMonitor emon1;  

ZMPT101B voltageSensor(35);

void setup()
{
  Serial.begin(9600);

  Serial.println("Calibrating... Ensure that no current flows through the sensor at this moment");
  delay(100);
  voltageSensor.calibrate();
  emon1.current(34, 30); 
  Serial.println("Done!");
}

void loop()
{

  float U = voltageSensor.getVoltageAC();
  if (U < 200){
    U = 0;
    Serial.println(String("U = ") + U + " V");
  }
  else{
    Serial.println(String("U = ") + U + " V");
  }

  delay(1000);
}

void loop()
{
  float Vrms = voltageSensor.getVoltageAC();
  float Irms = emon1.calcIrms(5588) - 1.3;  // Calculate Irms only

  if (Irms < 0){
    Irms = 0;
    Serial.println(String("Irms = ") + Irms + " A");               // Irms    
  }
  else {
    Serial.println(String("Irms = ") + Irms + " A");               // Irms   
  }

  if (Vrms < 200){
    Vrms = 0;
    Serial.println(String("Vrms = ") + Vrms + " V");
  }
  else{
    Serial.println(String("Vrms = ") + Vrms + " V");
  }
}