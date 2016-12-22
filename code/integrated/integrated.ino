#include <LowPower.h>


int mosfet = 9;  //pin 9

int interrupt_in = 3;  //pin 3, which can trigger an interrupt for waking up from power down.

volatile unsigned long push_time_us = 16*1000;

volatile int prev_interrupts;  //remember that I need to reject the first interrupt because the coil's inductance could trigger a self-perpetuating cycle



void setup() {
  // put your setup code here, to run once:


  prev_interrupts = 0;

  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,HIGH); //using pmos.  HIGH is off.

  delay(100);

  dac_setup();
    
  setup_ui();

  start();
  
  pinMode(interrupt_in,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

  

}

void loop() {
    // Allow wake up pin to trigger interrupt on low.
    attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);
    
    LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //118 uA.  but only on pins 2&3.  0&1 draw ~.5mA.

    detachInterrupt(digitalPinToInterrupt(interrupt_in));

  
}

void push()
{
  if(prev_interrupts > 0)
  {
    delayMicroseconds(1000); //allow to cross zero.  Hack since i'm using falling interrupts.
    digitalWrite(mosfet,LOW);
    delay_many_microseconds(push_time_us);
    digitalWrite(mosfet,HIGH);

    prev_interrupts = 0;
  }
  else
  {
    prev_interrupts ++;
    
  }

}

void start()
{
  for(int i = 0; i<10; i++)
  {
    delay(400);
    digitalWrite(mosfet,LOW);
    delay(300);
    digitalWrite(mosfet,HIGH);
    
    
  }

  
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

