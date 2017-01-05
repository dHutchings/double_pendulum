volatile int prev_interrupts;  //remember that I need to reject the first interrupt because the coil's inductance could trigger a self-perpetuating cycle

void setup_driver()
{

  prev_interrupts = 0;

  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,HIGH); //using pmos.  HIGH is off.
  delay(100); //just in case transients from MOSFET.

}

void  setup_sensing()
{
  
  pinMode(interrupt_in,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

}

void push()
{
  if(prev_interrupts > 0)
  {
    //need to impliment more inteligent re-cross detection.  We're going to set the threshold of V2 to 2V, busy-wait for a transition, set it back, and then push.
    //set_voltage(V2,2.0);
    //while(digitalRead(interrupt_in) == LOW)
    //{
    //  delayMicroseconds(1);
    //}
    //set_voltage(V2,1.9); //reset to original voltage threshold.

    delayMicroseconds(1); //allow to cross zero.
    //left in for future-proofing, but i may not need this.  uC requires significant time to wake from powerOff state, and I can change timing by also affecting the voltage threshold value.
    digitalWrite(mosfet,LOW);
    delay_many_microseconds(push_time_us + random(-random_time,random_time));
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

