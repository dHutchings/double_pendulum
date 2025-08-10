volatile int prev_pushes; //how many times have I pushed since I last chose a random number?
volatile long random_time; //random value

volatile float last_bemf; //the value of the most recent BEMF
void setup_driver()
{

  pinMode(drive_mosfet,OUTPUT);
  digitalWrite(drive_mosfet,LOW); //using nmos, pmos.  HIGH is on. LOW is off.
  delay(100); //just in case transients from MOSFET.

}

void  setup_zero_crossing_sensing()
{
  
  pinMode(interrupt_in,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push2,CHANGE);

}

void push2()
{
  if(digitalRead(interrupt_in)) //this is a RISING edge.  That's the first edge I get.  I don't want to do anyting here.
  {
    #if DEBUG
    digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on  
    #endif

    //This RISING edge means that The pulse is beginning.  I can no longer deepsleep (because that takes too long to boot up from), so let's go to the sleep that wakes up faster.
   digitalWrite(auto_timer_reset,HIGH);
   REASON_FOR_POWERUP = LIGHTSLEEP_WAIT;
  }
  else
  {
    #if DEBUG
    digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off  
    #endif

    digitalWrite(auto_timer_reset,LOW);
    //now, it's time to push...
    REASON_FOR_POWERUP = PUSH;
  }
}


void push()
{
  #if DEBUG
  digitalWrite(LED_BUILTIN_RX, LOW); // Turn RX LED on
  #endif
  
  if(random(0,100) > chance_no_push)  //sometimes, don't push.
  {
    //delayMicroseconds(1); //allow to cross zero.
    //this delay, is really should be on the order of MS, if we even use it!
    //This new (lower current, but slower) op-amps slew rate is so dang long that using the op-amp as a comparator is easy, but, may be bad.  We may want to go faster.
    //left in for future-proofing, but i may not need this.  uC requires significant time to wake from powerOff state, and I can change timing by also affecting the voltage threshold value.
    drive_MOS(HIGH); //unlike rev C, now we don't invert the signal in software (its in HW)..  So, drive_mosfet HIGH turns the coil on & lets the 555 timer know.
    long final_time = push_time_us + random_time;
  
    precise_idle(final_time); //Cuts overall 5V time average from 1.83 ma to 1.572 ma just by precise_idleing here, which cuts in 1/2 the current draw for this very small time.
    //just goes to show how much current the uC draws when fully up.
    //unfortunately, since this time is < 18 ms, I don't think there's a way to save even more power here...
    drive_MOS(LOW);
  
    prev_pushes ++;

    //According to the datasheet (Page 55), the bandgap reference takes 40 to 70uS to start up, and draws 10uA.
    //I havent had any success with saving power via BOD, (fuses?) - while the library's API suggests I can disable BOD
    //on my board its only possible via fuse settings.
    //according to datsheet, BOD on will also put on the Bandgap reference.
     
    last_bemf = measure_BEMF();
  
    if(prev_pushes > max_pushes)
    {
      prev_pushes = 0;
      random_time = random(-random_time_max,random_time_max);
      
    }
    
  }

  #if DEBUG_PRINTS
  Serial.println(last_bemf);
  delay(15);
  #else

  LowPower.powerStandby(SLEEP_15MS,ADC_OFF,BOD_ON); //low-power sleep, we need to sleep.  Reduces power from 3ma-ish to 1.99-ish on average (over continual running), really goes to show how much power the uC draws when its fully booted up.
  #endif

  //this would not work from an interrupt context, but since we call this from a loop it's fine.
  //since we just pushed, we can afford to do the powerDown (lowest power / wrong wake-up time) for just a bit
  //just to shave a bit more off of hte power.
  //this change reduced power from 1.594 ma to 1.5
    
  
  //put the BEMF sensince here... if I HAVE To delay for BEMF resetting reasons, I might as well use that time for prints.
  //it just works better if we delay for a bit, to avoid hysteresis. 
  //Also, I need to wait for hte curve to go more fully up. before resetting the BEMF, otherwise, I'm going to end the resetting while the curve is still R-L-C ing its way up, so it will immediately snap to too low of a value.

  clear_BEMF_sensing();
    

  #if DEBUG
  digitalWrite(LED_BUILTIN_RX, HIGH); // Turn RX LED off  
  #endif
}

void start_pendulum()
{
  for(int i = 0; i<10; i++)
  {
    
    precise_idle(400000);
    drive_MOS(HIGH);
    #if DEBUG
    digitalWrite(LED_BUILTIN_RX, LOW); // Turn RX LED on
    #endif

    precise_idle(300000);
    drive_MOS(LOW);
    #if DEBUG
    digitalWrite(LED_BUILTIN_RX, HIGH); // Turn RX LED off  
    #endif    
    
  }

  
}

//convienence function for both driving the MOSfET and letting the 555 timer know about it
void drive_MOS(bool value)
{
  digitalWrite(drive_mosfet,value);
  inform_restart_timer(value);
}
