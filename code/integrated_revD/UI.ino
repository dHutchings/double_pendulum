#include "constants.h"

void setup_ui()
{
  pinMode(faster,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(faster),speed_up,LOW);

  pinMode(slower,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(slower),slow_down,LOW);

  pinMode(start,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(start),start_sequence,LOW);

  //How to Turn off built-in RX / TX LEDs, to save power.
  //only relevant if there is USB serial traffic to begin with, which only shows up in debugging implimentations.

  pinMode(LED_BUILTIN_RX, OUTPUT); // RX LED - on right of pendulum as installed
  pinMode(LED_BUILTIN_TX, OUTPUT); // TX LED - on left of pendulum as installed
  digitalWrite(LED_BUILTIN_RX, HIGH); //turn off
  digitalWrite(LED_BUILTIN_TX, HIGH);
    


  for(int i = 600; i >= 150; i -= 150)
  {
    digitalWrite(LED_BUILTIN_RX, LOW); // Turn TX LED on  
    digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on
  
    delay(i);
    digitalWrite(LED_BUILTIN_RX, HIGH); // Turn TX LED off  
    digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off
    delay(i);
  }


  

}

void speed_up()
{
  wdt_disable(); //need to disable the Watchdog timer due to insane timing bug.

  //We have a powerStandby after pushing the MOSFET.  Seee #define USE_POST_PUSH_TIMEOUT
  //fine.  BUT, it usespowerStandby (one of the lighter sleeps) - as opposed to precise_idle or LowPower.Powerdown - for toming reasons.  This sleep mode delberately enables the WDT enabled, then un-sets it when it wakes back up.
  //if this interrupt were to fire wduring that USE_POST_PUSH_TIMEOUT, we'd have a problem
  //IF this interrupt were to fire during the USE_POST_PUSH_TIMEOUT, the process of the powerStandby un-setting the WDT never happens
  //and thus the WDT will fire, resetting the chip.
  //therefore, we need to un-set it ourselves manually
  
  REASON_FOR_POWERUP = UI;

  push_time_us = push_time_us + PUSH_SETTING_STEP;
  push_time_us = constrain(push_time_us,MIN_PUSH,MAX_PUSH);
  eeprom_current_speed();

  while(digitalRead(faster) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }

  #if GENERAL_DEBUG_PRINTS
  Serial.print("New Average push time: ");
  Serial.println(push_time_us);
  precise_idle(1000);
  #endif

  //then 100ms more to avoid debounce.
  //don't use any power-saving modes since Im in an ISR here
  delay_many_microseconds(100000);
}

void slow_down()
{
  wdt_disable();
  
  REASON_FOR_POWERUP = UI;

  push_time_us = push_time_us - PUSH_SETTING_STEP;
  push_time_us = constrain(push_time_us,MIN_PUSH,MAX_PUSH);
  eeprom_current_speed();

  while(digitalRead(slower) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }


  #if GENERAL_DEBUG_PRINTS
  Serial.print("New Average push time: ");
  Serial.println(push_time_us);
  precise_idle(1000);
  #endif


  //then 100ms more to avoid debounce.
  //don't use any power-saving modes since Im in an ISR here
  delay_many_microseconds(100000);
}

void start_sequence()
{
  wdt_disable();
  REASON_FOR_POWERUP = MANUAL_RESTART;

  while(digitalRead(start) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }

  push_time_us = NOMINAL_PUSH; //also, reset the speed.
  reset_eeprom();



  //then 100ms more to avoid debounce.
  delay_many_microseconds(100000);
  
}
