volatile int prev_interrupts;  //remember that I need to reject the first interrupt because the coil's inductance could trigger a self-perpetuating cycle

void setup_driver()
{

  prev_interrupts = 0;

  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,HIGH); //using pmos.  HIGH is off.
  delay(100); //just in case transients from MOSFET.

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

void start_pendulum()
{
  for(int i = 0; i<10; i++)
  {
    delay(400);
    digitalWrite(mosfet,LOW);
    delay(300);
    digitalWrite(mosfet,HIGH);
    
    
  }

  
}

