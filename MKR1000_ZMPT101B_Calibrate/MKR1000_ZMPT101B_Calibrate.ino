/* This code works with ESP8266 12E or Arduino and ZMPT101B AC voltage sensor up to 250 VAC 50/60Hz
 * It permits the measure of True RMS value of any AC signal, not only sinewave
 * The code uses the Sigma "Standard deviation" method and displays the value every "printPeriod"
 * check www.SurtrTech.com for more details
 */

#include <Filters.h>                            //Library to use

#define ZMPT101B 35                            //Analog input

float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 125/testFrequency;       // how long to average the signal, for statistist, changing this can have drastic effect
                                              // Test as you need

int RawValue = 0;     
float Volts_TRMS;     // estimated actual voltage in Volts

float intercept = 0;  // to be adjusted based on calibration testin
float slope = 1;      

/* How to get the intercept and slope? First keep them like above, intercept=0 and slope=1, 
 * also below keep displaying Calibrated and non calibrated values to help you in this process.
 * Put the AC input as 0 Volts, upload the code and check the serial monitor, normally you should have 0
 * if you see another value and it is stable then the intercept will be the opposite of that value
 * Example you upload first time and then you see a stable 1.65V so the intercept will be -1.65
 * To set the slope now you need to put the voltage at something higher than 0, and measure that using your reference TRMS multimeter
 * upload the new code with the new intercept and check the value displayed as calibrated values
 * Slope = (Measured values by multimeter)/(Measured values by the code)
 * Place your new slope and reupload the code, if you have problems with calibration try to adjust them both
 * or add a new line to calibrate further
 * Slope and intercept have nothing to do with the TRMS calculation, it's just an adjustement of the line of solutions
 */


unsigned long printPeriod = 1000; //Measuring frequency, every 1s, can be changed
unsigned long previousMillis = 0;

RunningStatistics inputStats; //This class collects the value so we can apply some functions

void setup() {
  Serial.begin(9600);    // start the serial port
  Serial.println("Serial started");
  inputStats.setWindowSecs( windowLength );
}

void loop() {
                 
     ReadVoltage();  //The only function I'm running, be careful when using with this kind of boards
                     //Do not use very long delays, or endless loops inside the loop
    
       
}

float ReadVoltage(){
    RawValue = analogRead(ZMPT101B);  // read the analog in value:
    inputStats.input(RawValue);       // log to Stats function
        
    if((unsigned long)(millis() - previousMillis) >= printPeriod) { //We calculate and display every 1s
      previousMillis = millis();   // update time
      
      Volts_TRMS = inputStats.sigma()* slope + intercept;
//      Volts_TRMS = Volts_TRMS*0.979;              //Further calibration if needed
      
      if (Volts_TRMS < 10){
        Volts_TRMS = 0;
          Serial.print("Non Calibrated: ");
          Serial.print("\t");
          Serial.print(inputStats.sigma()); 
          Serial.print("\t");
          Serial.print("Calibrated: ");
          Serial.print("\t");
          Serial.println(Volts_TRMS);
      }
        else{
          Serial.print("Non Calibrated: ");
          Serial.print("\t");
          Serial.print(inputStats.sigma()); 
          Serial.print("\t");
          Serial.print("Calibrated: ");
          Serial.print("\t");
          Serial.println(Volts_TRMS);
        }
  }
}