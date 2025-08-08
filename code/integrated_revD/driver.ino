volatile int prev_interrupts;  //remember that I need to reject the first interrupt because the coil's inductance could trigger a self-perpetuating cycle

volatile int prev_pushes; //how many times have I pushed since I last chose a random number?
volatile long random_time; //random value

volatile float last_bemf; //the value of the most recent BEMF
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

    
    delay(3); // we are pushing too early!  This ideal delay actually varies by how fast im going & the exact zero-crossing threshold, this is a hackkk.  If i'm going fast, this delay is too long  If i'm going slow, it's too short

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

      last_bemf = measure_BEMF();

      if(prev_pushes > max_pushes)
      {
        prev_pushes = 0;
        random_time = random(-random_time_max,random_time_max);
        
      }
      
      clear_BEMF_sensing();
    }
  }
  else
  {
    prev_interrupts ++;
    

    //put the BEMF sensince here... if I HAVE To delay for BEMF resetting reasons, I might as well use that time for prints.
    #if DEBUG_PRINTS
    Serial.println(last_bemf);
    #endif
    //it just works better if we delay for a bit, to avoid hysteresis. 
    //Also, I need to wait for hte curve to go more fully up. before resetting the BEMF, otherwise, I'm going to end the resetting while the curve is still R-L-C ing its way up, so it will immediately snap to too low of a value.
    LowPower.powerStandby(SLEEP_15MS,ADC_OFF,BOD_ON); //low-power sleep, we need to sleep.  Reduces power from 3ma-ish to 1.99-ish on average (over continual running), really goes to show how much power the uC draws when its fully booted up.
    
    clear_BEMF_sensing();
    
    
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

