#include <LowPower.h> //https://github.com/LowPowerLab/LowPower //LowPower_LowPower on the library manager
#include <TimerOne.h>

#include "pins.h"




//a series of constants that affect push time & randomness

volatile long push_time_us = 6.5*1000;  //push time.  It needs to be here, since multiple code segments deal with it.
volatile long random_time_max = 3000; //allow for +/- 4000uS push time randomness... used to prevent long-term cyclic oscilations.
volatile int max_pushes = 3; //number of times the pendulum will push untill is chooses a new random push time
volatile int chance_no_push = 10; //5% chance of not pushing this time, b/c randomness.  Set to -1 to turn off.

#define DEBUG false //set to true for debug-only TX / RX LEDs
#define DEBUG_PRINTS false //set to tue for debug-only prints.  This means the system cannot power down (USB issues).  ALSO, be aware that if the serial monitor window isnt open but we are still trying to send prints, the pendulum will stop working, too.

enum POWERUP_REASONS {
  PUSH,
  AUTO_RESTART,
  MANUAL_RESTART,
  UI,
  WAIT,
};

int REASON_FOR_POWERUP = WAIT; //a nice, neutral, way to start

void setup() {

  #if DEBUG
  Serial.begin(115200);
  delay(1000);
  Serial.println("Hello World");
  #endif

  setup_driver();
  setup_dac();
  setup_ui();
  start_pendulum();
  setup_zero_crossing_sensing(); //do this after the pendulum is started so we dont worry about any "triggering the drive" interrupt problems.
  setup_BEMF_sensing(); 
  setup_restart(); //do this AFTER the pendulum is started so we can clear the timer...



}

void loop() {
  switch(REASON_FOR_POWERUP)
  {
    case PUSH:
      //detachInterrupt before doing the starting, so we don't have "You started up 
      //detachInterrupt(digitalPinToInterrupt(interrupt_in));
      push();
      //attachInterrupt(digitalPinToInterrupt(interrupt_in),push2,FALLING);
      REASON_FOR_POWERUP = WAIT;
      break;
    case AUTO_RESTART:
      //detachInterrupt before doing the starting, so we can start cleanly w/out the startup sequence.
      //detachInterrupt(digitalPinToInterrupt(interrupt_in));
      start_pendulum();
      //attachInterrupt(digitalPinToInterrupt(interrupt_in),push2,FALLING);
      attachInterrupt(digitalPinToInterrupt(auto_timer_restart),auto_restart,LOW); //i tried falling, but because TIMER_RESTART is bound to Dio 7 right now, Falling doesn't work and I have to rely on LOW.
      //reattach the interrupt - I had to remove it to reduce the possibility of chain calling.
      REASON_FOR_POWERUP = WAIT;
      break;
    case MANUAL_RESTART:
      detachInterrupt(digitalPinToInterrupt(auto_timer_restart));
      start_pendulum();
      attachInterrupt(digitalPinToInterrupt(auto_timer_restart),auto_restart,LOW); //i tried falling, but because TIMER_RESTART is bound to Dio 7 right now, Falling doesn't work and I have to rely on LOW.
      
    case UI:
      REASON_FOR_POWERUP = WAIT;
      break; //do nothing, the UI interrupt handlers dealt with it
    case WAIT:
      break; //do nothing.  WAIT is a standard backup case.
    
  }

  // Allow wake up pin to trigger interrupt on low.

  //attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

  #if !DEBUG_PRINTS
  LowPower.powerStandby(SLEEP_FOREVER,ADC_OFF,BOD_ON); //500 uA-ish --> See the power tester for more info.  but only on pins 2&3.  0&1 draw ~.5mA.
  //LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //70 uA-ish --> See the power tester for more info.
  //powerStandby wakes up much faster than powerDown, likely because powerStandby keeps the crystal oscilator running (https://www.engineersgarage.com/reducing-arduino-power-consumption-sleep-modes/)
  #endif


  
}

void blank_ISR()
{
  
}

void precise_idle(unsigned long tim) //allow ability to delay up to 8.3 seconds highly precisely while in IDLE (17mA) - a power saving mode, but not the most aggressive.
{
  Timer1.initialize(tim);
  Timer1.attachInterrupt(blank_ISR); //have to attach an interrupt to a blank function, otherwise, it wont generate an interrupt that will get us out of idle.
  LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //About 17.1 mA
  Timer1.stop();  

}
void delay_many_microseconds(unsigned long tim) //allow ability to delay more than 16.3k uS... this does not have any power saving modes.
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
