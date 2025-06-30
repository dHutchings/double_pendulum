#include <LowPower.h> //https://github.com/LowPowerLab/LowPower //LowPower_LowPower on the library manager
#include "pins.h"




//a series of constants that affect push time & randomness

volatile long push_time_us = 10*1000;  //push time.  It needs to be here, since multiple code segments deal with it.
volatile long random_time_max = 4000; //allow for +/- 4000uS push time randomness... used to prevent long-term cyclic oscilations.
volatile int max_pushes = 3; //number of times the pendulum will push untill is chooses a new random push time
volatile int chance_no_push = 5; //5% chance of not pushing this time, b/c randomness. 

void setup() {


  setup_driver();
  setup_dac();
  setup_ui();
  start_pendulum();
  setup_sensing();

  

}

void loop() {

    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);
    
    LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //118 uA.  but only on pins 2&3.  0&1 draw ~.5mA.
    //remember that I've powered down timers here.  THerefore, timer0 (used for delay) may not work, avoid using delay().  use delayMicroseconds() instead.


  
}


void delay_many_microseconds(unsigned long tim) //allow ability to delay more than 16.3k uS.
{
  unsigned long time_left = tim;

  while(time_left > 0)
  {
    if(time_left > 16000)
    {
      delayMicroseconds(16000);
      time_left = time_left - 16000;
    }
    else
    {
      delayMicroseconds(int(time_left));
      time_left = 0;
    }
    
  }
  
}
