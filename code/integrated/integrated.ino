#include <LowPower.h>


int mosfet = 9;  //pin 9
int interrupt_in = 7;  //pin 3

volatile unsigned long push_time_us = 12*1000;

volatile int prev_interrupts;  //remember that I need to reject the first interrupt because the coil's inductance could trigger a self-perpetuating cycle
 

void setup() {
  // put your setup code here, to run once:


  prev_interrupts = 0;

  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,HIGH); //using pmos.  HIGH is off.

  delay(100);

  dac_setup();

  
  pinMode(interrupt_in,INPUT);
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

}

void loop() {
    // Allow wake up pin to trigger interrupt on low.
    //digitalWrite(is_awake,HIGH);
    //attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON); 
    //digitalWrite(is_awake,HIGH);
    // Disable external pin interrupt on wake up pin.
    //detachInterrupt(digitalPinToInterrupt(interrupt_in));
    
    // Do something here
    // Example: Read sensor, data logging, data transmission.
  
}

void push()
{
  if(prev_interrupts > 0)
  {
    delayMicroseconds(1000); //allow to cross zero.  Hack since i'm using falling interrupts.
    digitalWrite(mosfet,LOW);
    delayMicroseconds(push_time_us);
    digitalWrite(mosfet,HIGH);

    prev_interrupts = 0;
  }
  else
  {
    prev_interrupts ++;
    
  }

}

void start() //don't use for time being.
{
  for(int i = 0; i<10; i++)
  {
    digitalWrite(mosfet,LOW);
    delay(300);
    digitalWrite(mosfet,HIGH);
    delay(400);

    
  }

  
}

