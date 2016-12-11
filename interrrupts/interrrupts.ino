
volatile int mosfet = 9;  //pin 9

volatile int interrupt_in = 7;  //pin 3


volatile unsigned long push_time_us = 13*1000;

volatile int prev_interrupts;  //remember that I need to reject the first interrupt because the coil's inductance could trigger a self-perpetuating cycle
 

void setup() {
  // put your setup code here, to run once:

  prev_interrupts = 0;

  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,LOW);

  delay(100);
  
  pinMode(interrupt_in,INPUT);
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  
}

void push()
{
  if(prev_interrupts > 0)
  {
    delayMicroseconds(1000); //allow to cross zero.  Hack since i'm using falling interrupts.
    digitalWrite(mosfet,HIGH);
    delayMicroseconds(push_time_us);
    digitalWrite(mosfet,LOW);

    prev_interrupts = 0;
  }
  else
  {
    prev_interrupts ++;
    
  }
}

