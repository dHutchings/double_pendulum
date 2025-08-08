#include <LowPower.h> //https://github.com/LowPowerLab/LowPower //LowPower_LowPower on the library manager
#include "pins.h"
extern volatile int prev_interrupts;  //remember that I need to reject the first interrupt because the coil's inductance could trigger a self-perpetuating cycle




//a series of constants that affect push time & randomness

volatile long push_time_us = 7.5*1000;  //push time.  It needs to be here, since multiple code segments deal with it.
volatile long random_time_max = 4000; //allow for +/- 4000uS push time randomness... used to prevent long-term cyclic oscilations.
volatile int max_pushes = 3; //number of times the pendulum will push untill is chooses a new random push time
volatile int chance_no_push = 10; //5% chance of not pushing this time, b/c randomness.  Set to -1 to turn off.

#define DEBUG false //set to true for debug-only prints && TX / RX LEDs

void setup() {

  #if DEBUG
  Serial.begin(57600);
  delay(1000);
  Serial.println("Hello World");
  #endif

  setup_driver();
  setup_dac();
  setup_ui();
  start_pendulum();
  setup_zero_crossing_sensing();

  

}

void loop() {

    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

    
    LowPower.powerStandby(SLEEP_FOREVER,ADC_OFF,BOD_ON); //118 uA.  but only on pins 2&3.  0&1 draw ~.5mA.
    //powerStandby wakes up much faster than powerDown, likely because powerStandby keeps the crystal oscilator running (https://www.engineersgarage.com/reducing-arduino-power-consumption-sleep-modes/)
    
    //TODO: Measure the power delta here.  THis wasn't something Ive done before.  I dont think it's a lot of power.... but we really need to check.
    //LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //118 uA.  but only on pins 2&3.  0&1 draw ~.5mA.
    
    
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
