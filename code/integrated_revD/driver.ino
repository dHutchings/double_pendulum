#include "constants.h"


//purely internal variables
volatile int prev_pushes; //how many times have I pushed since I last chose a random number?


void setup_driver()
{

  pinMode(drive_mosfet,OUTPUT);
  digitalWrite(drive_mosfet,LOW); //using nmos, pmos.  HIGH is on. LOW is off.
  delay(100); //just in case transients from MOSFET.

}

void  setup_zero_crossing_sensing()
{
  
  pinMode(interrupt_in,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interrupt_in),interrupt_wake,CHANGE);

}

void interrupt_wake() //the helper that triggers the wake (either if I'm in deep, or light sleep).  It will figure out what the correct next step is based on the sign of the INT pin
{
  if(digitalRead(interrupt_in)) //this is a RISING edge.  That's the first edge I get.  I don't want to do anyting here.
  {
    #if DEBUG
    digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on  
    #endif

   REASON_FOR_POWERUP = LIGHTSLEEP_WAIT;
  }
  else
  {
    #if DEBUG
    digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off  
    #endif

    REASON_FOR_POWERUP = PUSH;
  }
}


void push()
{
  #if DEBUG
  digitalWrite(LED_BUILTIN_RX, LOW); // Turn RX LED on
  #endif
  
  if(random(0,100) > CHANCE_NO_PUSH)  //sometimes, don't push.
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
  
    if(prev_pushes > MAX_PUSHES)
    {
      prev_pushes = 0;

      #if SCALE_RANDOMNESS
      random_time = long(float(random(-RANDOM_AMOUNT,RANDOM_AMOUNT)) * float(push_time_us) / float(NOMINAL_PUSH)); 
      #else
      random_time = random(-RANDOM_AMOUNT,RANDOM_AMOUNT); 
      #endif
      //scale random time by the nominal max push time
      #if DEBUG_PRINTS
      final_time = push_time_us + random_time;

      Serial.print("Med: ");
      Serial.print(push_time_us);
      Serial.print("\tRand: ");
      Serial.print(random_time);
      Serial.print("\t Push Time: ");
      Serial.println(final_time);
      #endif
    }
    
  }

  #if DEBUG_PRINTS
  Serial.println(last_bemf);
  precise_idle(15000); //have to do this b/c we power standby for 15ms
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
  #if ! NO_PUSH
  digitalWrite(drive_mosfet,value);
  #endif
  inform_restart_timer(value);
}
