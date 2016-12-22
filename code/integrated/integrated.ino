#include <LowPower.h>


//PinDefs.
int interrupt_in = 3;  //pin 3, which can trigger an interrupt for waking up from power down.
int SCL_line = 6;
int SDA_line = 2;//clock is pin 6, data is pin 2.  Change pin2 to some unused Dio to free up another button
int faster = 0;  //pin 0
int slower = 1;  //pin 1
int mosfet = 9;  //pin 9





volatile unsigned long push_time_us = 16*1000;  //push time.  It needs to be here, since multiple code segments deal with it.




void setup() {
  // put your setup code here, to run once:


  setup_driver();
  setup_dac();
  setup_ui();

  start_pendulum();
  
  pinMode(interrupt_in,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

  

}

void loop() {

    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);
    
    LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //118 uA.  but only on pins 2&3.  0&1 draw ~.5mA.

    //detachInterrupt(digitalPinToInterrupt(interrupt_in));

  
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

