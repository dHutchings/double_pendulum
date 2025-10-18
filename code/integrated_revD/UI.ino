
void setup_ui()
{
  pinMode(faster,INPUT_PULLUP);
  pinMode(slower,INPUT_PULLUP);
  pinMode(start,INPUT_PULLUP);
  attach_interrupts();

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

void attach_interrupts()
{
  attachInterrupt(digitalPinToInterrupt(faster),speed_up,LOW);
  attachInterrupt(digitalPinToInterrupt(slower),slow_down,LOW);
  attachInterrupt(digitalPinToInterrupt(start),start_sequence,LOW);
}

void detach_interrupts()
{
  detachInterrupt(digitalPinToInterrupt(faster));
  detachInterrupt(digitalPinToInterrupt(slower));
  detachInterrupt(digitalPinToInterrupt(start));
}

int16_t type_of_UI_input() //2 if faster, 1 if slower, 0 else
{
  if(! digitalRead(faster))
  {
    return 2;
  }
  if(! digitalRead(slower))
  {
    return 1;
  }
  return 0;
}

void speed_up()
{
  //detach_interrupts();
  REASON_FOR_POWERUP = UI;

  //push_time_us = push_time_us + 500;
  //push_time_us = constrain(push_time_us,0,25000);


  /*
  while(digitalRead(faster) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }
  */




  //then 100ms more to avoid debounce.
  //don't use any power-saving modes since Im in an ISR here
  delay_many_microseconds(100000);
}

void slow_down()
{
  REASON_FOR_POWERUP = UI;
  //detach_interrupts();

  //push_time_us = push_time_us - 500;
  //push_time_us = constrain(push_time_us,0,25000);

  /*
  while(digitalRead(slower) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }
  */




  //then 100ms more to avoid debounce.
  //don't use any power-saving modes since Im in an ISR here
  //delay_many_microseconds(100000);
}

void start_sequence()
{
  REASON_FOR_POWERUP = MANUAL_RESTART;

  /*
  while(digitalRead(start) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }
  */


  //then 100ms more to avoid debounce.
  delay_many_microseconds(100000);
  
}
