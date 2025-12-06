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

float measure_BEMF()
{
  int val = analogRead(a_peak); 
  BEMF_History.add(float(val));

  #if SPEED_DEBUG_PRINTS
  Serial.print("Speed:");
  Serial.print(val);
  Serial.print(",Avg:");
  Serial.print(BEMF_History.getFastAverage());
  Serial.print(",Diff:");
  Serial.println(BEMF_History.getMaxInBuffer() - BEMF_History.getMinInBuffer());
  #endif

  //if there isn't a lot of variation in my hisotyr
  //and if I am going fast.
  if( (BEMF_History.getMaxInBuffer() - BEMF_History.getMinInBuffer()) < 70 && BEMF_History.getFastAverage() < 150)
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
  


  return float(val);

  //float v_peak = float(val)*float(5000)/float(1024); //I like millivolts, not volts.
  //return v_peak;
  //return 2000 - v_peak; //Probably, in final controls mode, I would like the sign reversed so larger is faster.  But right now, return the number I can confirm with o-scope
}

//This is all we need to reset the BEMF measurement circuit, but, it is incumbent on whoever calls this function to make sure it's a good time to reset it!
void clear_BEMF_sensing()
{
  pinMode(a_peak_rst,OUTPUT);
  digitalWrite(a_peak_rst,HIGH); //directly load 5V into the capacitor
  delayMicroseconds(5);
  pinMode(a_peak_rst,INPUT); //turn the DIO back into an input, so its floating and the PEAK_RST line is now controlled by the op=amps.
}
