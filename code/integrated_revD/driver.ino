volatile int prev_interrupts;  //remember that I need to reject the first interrupt because the coil's inductance could trigger a self-perpetuating cycle

volatile int prev_pushes; //how many times have I pushed since I last chose a random number?
volatile long random_time; //random value

void setup_driver()
{

  prev_interrupts = 0;

  pinMode(drive_mosfet,OUTPUT);
  digitalWrite(drive_mosfet,LOW); //using nmos, pmos.  HIGH is on. LOW is off.
  delay(100); //just in case transients from MOSFET.

}

void  setup_zero_crossing_sensing()
{
  
  pinMode(interrupt_in,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

}


void push()
{
  #if DEBUG
  digitalWrite(LED_BUILTIN_RX, LOW); // Turn RX LED on
  //Serial.println(prev_interrupts);
  #endif

  if(prev_interrupts > 0)
  {
    //need to impliment more inteligent re-cross detection.  We're going to set the threshold of V2 to 2V, busy-wait for a transition, set it back, and then push.
    //set_voltage(V2,2.0);
    //while(digitalRead(interrupt_in) == LOW)
    //{
    //  delayMicroseconds(1);
    //}
    //set_voltage(V2,1.9); //reset to original voltage threshold.

    
    delay(5); // we are pushing too early!  This ideal delay actually varies by how fast im going & the exact zero-crossing threshold, this is a hackkk.  If i'm going fast, this delay is too long  If i'm going slow, it's too short

    if(random(0,100) > chance_no_push)  //sometimes, don't push.
    {
      //delayMicroseconds(1); //allow to cross zero.
      //this delay, is really should be on the order of MS, if we even use it!
      //This new (lower current, but slower) op-amps slew rate is so dang long that using the op-amp as a comparator is easy, but, may be bad.  We may want to go faster.
      //left in for future-proofing, but i may not need this.  uC requires significant time to wake from powerOff state, and I can change timing by also affecting the voltage threshold value.
      digitalWrite(drive_mosfet,HIGH); //unlike rev C, now we don't invert the signal in software (its in HW)..  So, drive_mosfet HIGH turns the coil on.

      delay_many_microseconds(push_time_us + random_time);
      digitalWrite(drive_mosfet,LOW);
  
      prev_interrupts = 0;
      prev_pushes ++;
      
      if(prev_pushes > max_pushes)
      {
        prev_pushes = 0;
        random_time = random(-random_time_max,random_time_max);
        
      }
    }
  }
  else
  {
    prev_interrupts ++;
    
  }

  #if DEBUG
  digitalWrite(LED_BUILTIN_RX, HIGH); // Turn RX LED off  
  #endif
}

void start_pendulum()
{
  for(int i = 0; i<10; i++)
  {
    delay_many_microseconds(400000);
    digitalWrite(drive_mosfet,HIGH);

    #if DEBUG
    digitalWrite(LED_BUILTIN_RX, LOW); // Turn RX LED on
    #endif

    delay_many_microseconds(300000);
    digitalWrite(drive_mosfet,LOW);
    #if DEBUG
    digitalWrite(LED_BUILTIN_RX, HIGH); // Turn RX LED off  
    #endif    
    
  }

  
}

