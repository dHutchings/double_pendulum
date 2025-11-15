#include <SPI.h>
#include <LTC1661.h>
#include <LowPower.h>


int chipSelect = 10;

LTC1661 dac(chipSelect);    //creats an instance with chipSelect as CS Pin


#define DEBUG false //TRUE to get prints.
#define SHORT_SLEEP false //TRUE to get rapid ramping behaviour.  Easier to debug, but inaccurate long-term power measurements.

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello World");
  delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:

  for(int i = 0; i <= 1023; i++)
  {
     dac.loUp(i, 1023-i);  //send values and update both OUTPUTS
     Serial.print("Both:\t");
     Serial.println(5*float(i)/float(1024));     

     #if DEBUG
     delay(15);
     #elif SHORT_SLEEP
     LowPower.powerDown(SLEEP_15MS,ADC_OFF,BOD_OFF);
     #else 
     LowPower.powerDown(SLEEP_500MS,ADC_OFF,BOD_OFF);
     #endif
  }

  dac.loUpB(512);
  for(int i = 0; i <= 1023; i++)
  {
     dac.loUpA(i);  //send values to DAC A
     Serial.print("A:\t");
     Serial.println(5*float(i)/float(1024));
     #if DEBUG
     delay(15);
     #elif SHORT_SLEEP
     LowPower.powerDown(SLEEP_15MS,ADC_OFF,BOD_OFF);
      #else 
     LowPower.powerDown(SLEEP_500MS,ADC_OFF,BOD_OFF);
     #endif
  }
  
  dac.loUpA(512);
  for(int i = 0; i <= 1023; i++)
  {
     dac.loUpB(i);  //send values to DAC A
     Serial.print("B:\t");
     Serial.println(5*float(i)/float(1024));
     #if DEBUG
     delay(15);
     #elif SHORT_SLEEP
     LowPower.powerDown(SLEEP_15MS,ADC_OFF,BOD_OFF);
     #else 
     LowPower.powerDown(SLEEP_500MS,ADC_OFF,BOD_OFF);
     #endif
   }
  

}
