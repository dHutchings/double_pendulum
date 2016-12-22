
int faster = 0;  //pin 0
int slower = 1;  //pin 1

void setup_ui()
{
  pinMode(faster,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(faster),speed_up,LOW);

  pinMode(slower,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(slower),slow_down,LOW);

  
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

