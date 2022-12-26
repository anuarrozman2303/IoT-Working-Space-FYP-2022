#include "ZMPT101B.h"

ZMPT101B voltageSensor(35);

void setup()
{
  Serial.begin(9600);

  Serial.println("Calibrating... Ensure that no current flows through the sensor at this moment");
  delay(100);
  voltageSensor.calibrate();
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