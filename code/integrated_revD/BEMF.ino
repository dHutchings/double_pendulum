#include "RunningAverage.h"
RunningAverage BEMF_History(10);  //gets data sample of last 10 BEMFs. / when I should have pushed


void setup_BEMF_sensing()
{
  pinMode(a_peak_rst,INPUT); // dont reset the A_peak_sensing
  pinMode(a_peak,INPUT);
  DIDR2 |= _BV(ADC8D);
  //per the datasheet, 7.8.6. we need to explicitly disable the digital input buffer since we're using it as an analog port.
  //This change makes little difference at rest .. but it's thing to do according to the datasheet.

  BEMF_History.fillValue(1024,10);

}

//returns BEMF in volts
//as if it were measured from an oscilliscope
//Which means that the closer to 0 it is, the faster I am going.

//ALSO does the control on BEMF voltage (should this be done elsewhere for code clarity...?)
float measure_BEMF()
{
  float bemf_measurement = analogRead(a_peak) *5.0/1023.0 ; //put it into volts
  BEMF_History.add(bemf_measurement); 

  
  #if SPEED_DEBUG_PRINTS
  Serial.print("Speed:");
  Serial.print(bemf_measurement);
  Serial.print(",Avg:");
  Serial.print(BEMF_History.getFastAverage());
  Serial.print(",Diff:");
  Serial.println(BEMF_History.getMaxInBuffer() - BEMF_History.getMinInBuffer());
  #endif
  

  //if there isn't a lot of variation in my hisotyr
  //and if I am going fast.
  if( (BEMF_History.getMaxInBuffer() - BEMF_History.getMinInBuffer()) < 0.342 && BEMF_History.getFastAverage() < 0.732)
  {
    
    #if SPEED_DEBUG_PRINTS
    Serial.println("Detected Swinging In perpetual loop, straight arm");
    #endif
    
    
    #if OUTSTRECHED_ARM_SLOWING
    if(NUM_PUSHES_TO_SKIP == 0)
    {
      //Slow down the pendulum just a bit.
      //but, Just do it the first time.
      push_time_us = push_time_us - PUSH_SETTING_STEP;
      push_time_us = constrain(push_time_us,MIN_PUSH,MAX_PUSH);
      eeprom_current_speed();
    }
    
    NUM_PUSHES_TO_SKIP = 6; //skip the next 6 pushes to let the pendulum restart.
    //It is probable that I triggered MANY times, not just one, so effectively I skip for each loop I sense I'm too fast + 6 more.
    //That works.  That way, I will always skip untill I'm slow enough, then skip just a bit more.



    #endif

  }
  


  return bemf_measurement;
}

//This is all we need to reset the BEMF measurement circuit, but, it is incumbent on whoever calls this function to make sure it's a good time to reset it!
void clear_BEMF_sensing()
{
  pinMode(a_peak_rst,OUTPUT);
  digitalWrite(a_peak_rst,HIGH); //directly load 5V into the capacitor
  delayMicroseconds(5);
  pinMode(a_peak_rst,INPUT); //turn the DIO back into an input, so its floating and the PEAK_RST line is now controlled by the op=amps.
}
