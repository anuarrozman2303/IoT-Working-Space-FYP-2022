// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3
#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor emon1;                   // Create an instance

void setup()
{
  Serial.begin(9600);
  emon1.current(34, 60.606);             // Current: input pin, calibration. (100 ÷ 0.050) ÷ 150 Ohm = 13.33
}

void loop()
{
  int val;
  val = analogRead(34);
  Serial.println(val);
  unsigned long previousMillis = millis();
  int count = 0;
  double Irms = 0;
  while ((millis() - previousMillis) < 1000)
  {
    Irms += emon1.calcIrms(1480);  // Calculate Irms only
    count++;
  }
  Irms = Irms/count;
  Serial.print(Irms * 241.0);
  Serial.print(" ");
  Serial.print("W ");
  Serial.print(Irms);
  Serial.print(" ");
  Serial.print("Irms ");

}