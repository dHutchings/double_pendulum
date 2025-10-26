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

  pinMode(LED_BUILTIN_RX, OUTPUT); // TX LED
  pinMode(LED_BUILTIN_TX, OUTPUT); // TX LED
  digitalWrite(LED_BUILTIN_RX, LOW); // Turn TX LED on  
  digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on

  delay(500);
  digitalWrite(LED_BUILTIN_RX, HIGH); // Turn TX LED off  
  digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off

}

void speed_up()
{
  REASON_FOR_POWERUP = UI;

  push_time_us = push_time_us + PUSH_SETTING_STEP;
  push_time_us = constrain(push_time_us,MIN_PUSH,MAX_PUSH);
  eeprom_current_speed();

  while(digitalRead(faster) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }

  #if DEBUG_PRINTS
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
  REASON_FOR_POWERUP = UI;

  push_time_us = push_time_us - PUSH_SETTING_STEP;
  push_time_us = constrain(push_time_us,MIN_PUSH,MAX_PUSH);
  eeprom_current_speed();

  while(digitalRead(slower) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }


  #if DEBUG_PRINTS
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
