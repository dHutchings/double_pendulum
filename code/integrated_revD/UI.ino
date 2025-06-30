
void setup_ui()
{
  pinMode(faster,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(faster),speed_up,LOW);

  pinMode(slower,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(slower),slow_down,LOW);

  pinMode(start,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(start),start_sequence,LOW);
  
}

void speed_up()
{
  
  push_time_us = push_time_us + 1000;
  push_time_us = constrain(push_time_us,0,25000);

  while(digitalRead(faster) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }


  //then 100ms more to avoid debounce.
  delay_many_microseconds(100000);
}

void slow_down()
{
    push_time_us = push_time_us - 1000;
    push_time_us = constrain(push_time_us,0,25000);

    while(digitalRead(slower) == LOW)
    {
      delayMicroseconds(1000); //keep waiting for unpress.  
    }


    //then 100ms more to avoid debounce.
    delay_many_microseconds(100000);

    
}

void start_sequence()
{
  while(digitalRead(start) == LOW)
  {
    delayMicroseconds(1000); //keep waiting for unpress.  
  }


  //then 100ms more to avoid debounce.
  delay_many_microseconds(100000);
  detachInterrupt(digitalPinToInterrupt(interrupt_in));

  start_pendulum();
  
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);
}

