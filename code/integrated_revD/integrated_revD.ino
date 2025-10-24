#include <LowPower.h> //https://github.com/LowPowerLab/LowPower //LowPower_LowPower on the library manager
#include <TimerOne.h>

#include "pins.h"




//a series of constants that affect push time & randomness

static long nominal_push_time_us = 5500; //nominal push time, fixed value that we always use.
volatile long push_time_us = nominal_push_time_us;  //actual push time.  It needs to be here, since multiple code segments deal with it.  Can be adjusted up and down via button presses
volatile long random_time_max = 3000; //allow for +/- 3000uS push time randomness... used to prevent long-term cyclic oscilations.
#define SCALE_RANDOMNESS true //set to true to enable the amount of randomness to be scaled up and down by how much we are over or underpushing, not just some nominal value.

//the real random push time will be scaled up or down from this value.  EG the nominal push time is 5500uS.  If it's half of that (2750 uS), the nominal random time max could cause a negative push time.
//instead

volatile int max_pushes = 3; //number of times the pendulum will push untill is chooses a new random push time
volatile int chance_no_push = 10; //5% chance of not pushing this time, b/c randomness.  Set to -1 to turn off.

#define DEBUG false //set to true for debug-only TX / RX LEDs.  TX LED (left?) is generally around the drive interrupt trigger times, RX is around the MOSFET driving.
#define DEBUG_PRINTS false //set to tue for debug-only prints.  This means the system cannot power down (USB issues).  ALSO, be aware that if the serial monitor window isnt open but we are still trying to send prints, the pendulum will stop working, too.

enum POWERUP_REASONS {
  PUSH,
  AUTO_RESTART,
  MANUAL_RESTART,
  UI,
  DEEPSLEEP_WAIT, //This uses the powerDown mode
  LIGHTSLEEP_WAIT, //This uses the powerStandby mode
};

int REASON_FOR_POWERUP = DEEPSLEEP_WAIT; //a nice, neutral, way to start

void setup() {

  #if DEBUG_PRINTS
  Serial.begin(115200);
  delay(5000);
  Serial.println("Hello World");
  delay(1000);
  #else
  //various powersaving which trims 29uA off the power consumption, Praise be to https://forum.arduino.cc/t/power-consumption-of-atmega32u4-during-sleep-power-down-higher-than-expected/685915/28?page=2
  //these probably kill serial prints, so only run them if we need the debug prints.
  USBCON |= (1 << FRZCLK);             // Freeze the USB Clock              
  PLLCSR &= ~(1 << PLLE);              // Disable the USB Clock (PPL) 
  USBCON &=  ~(1 << USBE  );           // Disable the USB  
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
      //detachInterrupt before doing the starting, so we don't have the secondary flyback pulses interrupt our handing of this.
      detachInterrupt(digitalPinToInterrupt(interrupt_in));
      push();
      attachInterrupt(digitalPinToInterrupt(interrupt_in),interrupt_wake,CHANGE);
      REASON_FOR_POWERUP = DEEPSLEEP_WAIT;
      break;
    case AUTO_RESTART:
      //detachInterrupt before doing the starting, so we can start cleanly w/out the startup sequence.
      detachInterrupt(digitalPinToInterrupt(auto_timer_restart)); //i tried falling, but because TIMER_RESTART is bound to Dio 7 right now, Falling doesn't work and I have to rely on LOW.
      detachInterrupt(digitalPinToInterrupt(interrupt_in));
      
      //Don't need to detach reattach interrupts - all the interrupt does is change a flag now (which I will shortly change, down here)
      start_pendulum();

      attachInterrupt(digitalPinToInterrupt(auto_timer_restart),auto_restart,LOW); //i tried falling, but because TIMER_RESTART is bound to Dio 7 right now, Falling doesn't work and I have to rely on LOW.
      attachInterrupt(digitalPinToInterrupt(interrupt_in),interrupt_wake,CHANGE);

      REASON_FOR_POWERUP = DEEPSLEEP_WAIT;
      break;
    case MANUAL_RESTART:
      detachInterrupt(digitalPinToInterrupt(auto_timer_restart));
      start_pendulum();
      attachInterrupt(digitalPinToInterrupt(auto_timer_restart),auto_restart,LOW); //i tried falling, but because TIMER_RESTART is bound to Dio 7 right now, Falling doesn't work and I have to rely on LOW.
      
    case UI:
      REASON_FOR_POWERUP = DEEPSLEEP_WAIT;
      break; //do nothing, the UI interrupt handlers dealt with it
    case DEEPSLEEP_WAIT:
      break;
    case LIGHTSLEEP_WAIT:
      break;    
  }

  //depending on the sleep mode, do different things here.
  if(REASON_FOR_POWERUP == DEEPSLEEP_WAIT)
  {
    #if !DEBUG_PRINTS //need this otherwise prints wont go through
    LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //70 uA-ish --> See the power tester for more info.
    #endif
  }
  else if(REASON_FOR_POWERUP == LIGHTSLEEP_WAIT)
  {
    #if !DEBUG_PRINTS //need this otherwise prints wont go through
    LowPower.powerStandby(SLEEP_FOREVER,ADC_OFF,BOD_ON); //500 uA-ish --> See the power tester for more info.  but only on pins 2&3.  0&1 draw ~.5mA.    
    //powerStandby wakes up much faster than powerDown, likely because powerStandby keeps the crystal oscilator running (https://www.engineersgarage.com/reducing-arduino-power-consumption-sleep-modes/)
    #endif

  }


  
}

void blank_ISR()
{
  
}

void precise_idle(long tim) //allow ability to delay up to 8.3 seconds highly precisely while in IDLE (17mA) - a power saving mode, but not the most aggressive.
{
  if(tim <= 0)
  {
    return;
  }
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
