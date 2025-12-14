#include <LowPower.h>

#include <TimerOne.h> //use internal timer for precise wake-ups (only when in idle).  See quickstart 57.  It only works for periods < 8.3 seconds.


//code that will test power consumption of CPU under various conditions.

int mosfet = 10;  //pin 9
int interrupt_in = 3;  //pin 3.  Was pin 7, but pin 7 maps to interupt #4, which doesn't have the ability to wake from power off via FALLING or CHANGE.

volatile unsigned long push_time_us = 12*1000;
 
bool MEASURE_PULSEIN_DURATION = false; //A setting, set to TRUE to measure pulse in duration - Except that this will force us to use LowPower.Idle (for now) - wastes power.  PLUS, LEDS blink and prints go out.
bool USE_LEDS = true; //A setting, wastes power, so helpful to easily switch off.

long event_time = 0;
bool fired_push = false;

int timer1_to_us = 0;


//Deduced from https://forum.arduino.cc/t/timer1-prescaler/585241
int get_prescaler()
{
  int prescaler_settings = (TCCR1B & 0b111);
  switch (prescaler_settings)
  {
    case 1:
      return 1;
    case 2:
      return 8;
    case 3:
      return 64;
    case 4:
      return 256;
    case 5:
      return 1024;
    default:
      return 0;
  }
}

void setup() {
  // put your setup code here, to run once:

  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,LOW);

  
  pinMode(interrupt_in,INPUT); //hook up to function generator.
  //But be careful, can only wake up on changes to interupts 0:3, pins 3,2,1,0.

  
  attachInterrupt(digitalPinToInterrupt(interrupt_in),push,CHANGE);

  pinMode(LED_BUILTIN_RX, OUTPUT); // TX LED
  pinMode(LED_BUILTIN_TX, OUTPUT); // TX LED
  digitalWrite(LED_BUILTIN_RX, LOW); // Turn TX LED on
  digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on
  delay(5000); //time for the uC to be booted, making it easier to reprogram

  digitalWrite(LED_BUILTIN_RX, HIGH); // Turn TX LED off  
  digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off

  //44.25
  
  Serial.begin(9600);
  Serial.println("Hello World");
  delay(5000);


  /*
  Serial.println(DIDR0);
  Serial.println(DIDR1);
  Serial.println(DIDR2);
  */

  /*
   * The datasheet says to do this "If the On-chip debug system is enabled by the OCDEN Fuse...
   * Unfortunately, I can't check that fuse.  Tests suggest that this change saves 0, so it does nothing.
   * so it must not be enabeled.
  int desired_mcucr = MCUCR | _BV(JTD);
  //MCUCR = desired_mcucr;
  //MCUCR = desired_mcucr;
  // In order to avoid unintentional disabling or enabling of the JTAG interface, a timed
  //sequence must be followed when changing this bit: The application software must write this bit to the desired
  //value twice within four cycles to change its value.
  
  MCUCR = MCUCR | (1<<7);
  MCUCR = MCUCR | (1<<7);
  Serial.println(MCUCR);
  */

  //Uncomment to try the Idle self-waking OR the pulse-timer stuff.
  /*
  Timer1.attachInterrupt(push);
  Timer1.initialize(8388480 ); //8388480  is from https://deepbluembedded.com/arduino-timerone-library/#:~:text=Timer1.setPeriod(period)&text=The%20minimum%20period%20or%20highest,frequencies%20and%20duty%20cycles%20simultaneously.
  */

  //Uncommend to try the pulse-measuring stuff

  if(MEASURE_PULSEIN_DURATION)
  {
    Timer1.initialize(8388480); //max time period, 8.3 seconds.  This is basically the longest it can run, i.e. the longest duration (lowest precision) measurement.  But, get_prescaler() will let us set it shorter if we wanted.
    timer1_to_us = get_prescaler() / (F_CPU / 1000 / 1000);  //enable us to do the time measurement.
    Serial.print("pre-scaler");
    Serial.println(get_prescaler());
    Serial.print("Each Tic is X uS:");
    Serial.println(timer1_to_us);
  }  


}


void loop() {

    //I'm in loop once We're done with the interrupt.
    //Twiddle some lights
    //And then immediately go back to sleep
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.

    //There are LOTS of reasons I can have interrupts fired when I try to leave timers, USB on.
    //Any of those will wake me.
    //If its any of htose - or more precisely, any of the NOT-externally fired push() interruots.
    //I should just put the uC back to sleep ASAP and skip all the funny buisness.
    if(fired_push)
    {

      //the timer1's t=0 is MOSFET (YELOW) being written low.


      //the timer1's tfinal is the INPUT interrupt_in (Yellow) FALLING.

      detachInterrupt(digitalPinToInterrupt(interrupt_in)); //detach interrupt immediately, as a form of debounce protection

      if(MEASURE_PULSEIN_DURATION)
      {
    
        Serial.print("Time Since last fired Push (interupt LOW) (Timer1 Cycles): "); //This wont print when USB gets shut down, more than the first time...
        Serial.print(event_time);
        long time_in_us = event_time*timer1_to_us; 
        Serial.print("\t uS:");
        Serial.print(time_in_us);
        Serial.print("\t Hz:");
        Serial.println(float(1000000)/float(time_in_us));
      }
      
      //Serial.println(event_time/(F_CPU/8));
  
      //Having Delays here can veyr much throw o-scope measurements off - makes sense, I'm not attached quickly enough the next time through the loop/
      if(USE_LEDS)
      {
        digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on  
        delay(100);
        digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off  
        delay(100);
      }
      else
      {
        delay(200);
      }
  
      attachInterrupt(digitalPinToInterrupt(interrupt_in),push,CHANGE); //reattach interrupt so I can wake back up
      //pin 0,1,2,3 can do CHANGE

      fired_push = false;

    }

    //31mA when no power saving is applied.

    //Uncomment to try the Idle self-waking
    /*
    //do NOT use restart (or start), it generates unwanted interrupts.
    //Best you can do is resume - which doesnt reset the counter - which is why it's important to cal stop as the first step of the interrupt.
    //unfortunately, this means that if we stop sleeping due to an external interrupt, the next time we wake from an internal interupt, we won't sleep for the right time.
    Timer1.resume(); 
    
    
    LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //16.8 mA...
    //note timer 0 is off, you will need to use delayMicroseconds
    //note timer 0 CANNOT be turned on, since that'd trigger an interrupt every 1ms, and foil the plan.
    //timer 3 works pretty much the same way and draws the same amount of power as timer1.
    */

    /* Duration Data data */
    //BOD_ON makes no difference.
    //LowPower.powerDown(SLEEP_15MS,ADC_OFF,BOD_OFF); // takes about 21.15 ms to fully execute 
    //LowPower.powerDown(SLEEP_30MS,ADC_OFF,BOD_OFF); // takes about 41.90 ms to fully execute 
    //LowPower.powerDown(SLEEP_60MS,ADC_OFF,BOD_OFF); // takes about 77.50 ms to fully execute 
    //LowPower.powerDown(SLEEP_120MS,ADC_OFF,BOD_OFF); // takes about 149.8 ms to fully execute 
    //LowPower.powerDown(SLEEP_250MS,ADC_OFF,BOD_OFF); // takes about 294.0 ms to fully execute     
    //LowPower.powerDown(SLEEP_500MS,ADC_OFF,BOD_OFF); // takes about 582.0 ms to fully execute   


    //LowPower.powerSave(SLEEP_15MS,ADC_OFF,BOD_OFF,TIMER2_OFF);  //Takes about 21.10 ms to fully execute
    //LowPower.powerSave(SLEEP_30MS,ADC_OFF,BOD_OFF,TIMER2_OFF);  //Takes about 41.90 ms to fully execute
    //LowPower.powerSave(SLEEP_60MS,ADC_OFF,BOD_OFF,TIMER2_OFF); // takes about 77.70 ms to fully execute
    //LowPower.powerSave(SLEEP_120MS,ADC_OFF,BOD_OFF,TIMER2_OFF); // takes about 149.8 ms to fully execute
    //LowPower.powerSave(SLEEP_250MS,ADC_OFF,BOD_OFF,TIMER2_OFF); // takes about 294.0 ms to fully execute
    //LowPower.powerSave(SLEEP_500MS,ADC_OFF,BOD_OFF,TIMER2_OFF); // takes about 582.0 ms to fully execute    

    //LowPower.powerStandby(SLEEP_15MS,ADC_OFF,BOD_OFF);  //Tales 18.10 ms to fully execute
    //LowPower.powerStandby(SLEEP_30MS,ADC_OFF,BOD_OFF);  //Tales 36.10 ms to fully execute
    //LowPower.powerStandby(SLEEP_60MS,ADC_OFF,BOD_OFF);  //Tales 72.30 ms to fully execute
    //LowPower.powerStandby(SLEEP_120MS,ADC_OFF,BOD_OFF);  //Tales 144.6 ms to fully execute
    //LowPower.powerStandby(SLEEP_250MS,ADC_OFF,BOD_OFF);  //Tales 289.0 ms to fully execute
    //LowPower.powerStandby(SLEEP_500MS,ADC_OFF,BOD_OFF);  //Tales 578.0 ms to fully execute

    //LowPower.idle(SLEEP_15MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //Takes 18.10 ms to fully execute
    //LowPower.idle(SLEEP_30MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //Takes 36.20 ms to fully execute
    //LowPower.idle(SLEEP_60MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //Takes 72.30 ms to fully execute
    //LowPower.idle(SLEEP_120MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //Takes 144.6 ms to fully execute
    //LowPower.idle(SLEEP_250MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //Takes 289.0 ms to fully execute
    //LowPower.idle(SLEEP_500MS, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //Takes 578.0 ms to fully execute

    //precise_idle(15000); //takes exactly 15ms.
    //precise_idle(30000); //takes exactly 30ms.
    //precise_idle(32000); //takes exactly 32ms.
    //precise_idle(33000); //takes exactly 33 ms.
    //precise_idle(33*1000);  //takes 8.4 seconds!
    

    //precise_idle(150000); //takes exactly 150ms
    //precise_idle(150*1000); //takes 19.5 ms.
    //CONCLUSION: DON"T DO MULTIPLICATION HERE, HARDCODE THE NUMBER (probably b/c of the inline thing...)


    //It looks powerStandby and Idle are 20% longer than nominal (powerDown and powerSave are even longer due to that extra 5ms)
    //Basically, powerDown and powerSave typically take 5.5 ms more than the powerStandby and Idle - thats probably EITHER the oscilator start-up time OR the Fuse settings.
    //Except the 15ms (shortest) setting, which takes shorter


    //digitalWriteHIGH, digitalWriteLOW back to back takes only 5.4 us.

    //digitalWrite(mosfet,HIGH);
    //LowPower.powerDown(SLEEP_30MS,ADC_OFF,BOD_ON); // takes about 21.15 ms to fully execute 

    //precise_idle(5000000); //About 17.1 mA

    //digitalWrite(mosfet,LOW);
  
    //For some reason, BOD_ON results in less power dissipation.  Wierd.

    

    /* power & Wake-up time data */

    //Praise be to https://forum.arduino.cc/t/power-consumption-of-atmega32u4-during-sleep-power-down-higher-than-expected/685915/28?page=2
    //These 3 commands bring us from 73.72 down to 44.63 uA for the powerDown command.  Major success, everyone is happy!
    //this freezes various USB clocks which apparently were left on but the datasheet does explicitly call out as being able to turn off.
    //safe to say, serial prints aren't working if we run these.
    //USBCON |= (1 << FRZCLK);             // Freeze the USB Clock              
    //PLLCSR &= ~(1 << PLLE);              // Disable the USB Clock (PPL) 
    //USBCON &=  ~(1 << USBE  );           // Disable the USB 


    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON); // Does not keep micros OR millis alive.
    //LowPower.idle(SLEEP_FOREVER, ADC_ON, TIMER4_ON, TIMER3_ON, TIMER1_ON, TIMER0_ON, SPI_ON, USART1_ON, TWI_ON, USB_ON); //millis or micros also not running
    //Serial.println(TCNT1);

    //LowPower.idle(SLEEP_500MS, ADC_ON, TIMER4_ON, TIMER3_ON, TIMER1_ON, TIMER0_OFF, SPI_ON, USART1_ON, TWI_ON, USB_ON); //Not this, either.
    //Serial.println(TCNT1);

    


    
    //LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF); //73.72 uA on the USB C APM.  118 uA on the old APM.  but only on pins 2&3.  0&1 draw ~.5mA... and on the new APM there isn't any pin 1 vs pin 3 performance difference.  Takes 3.65 ms to wake up.
    //LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //108.15 on the USB C APM  227 uA on the Old APM.  But be careful, can only wake up on changes to interupts 0:3, pins 3,2,1,0.  

    //LowPower.powerSave(SLEEP_FOREVER,ADC_OFF,BOD_OFF,TIMER2_OFF);  //108.35 ish on the USB C APM  Takes 3.63 ms to wake up.
    //LowPower.powerSave(SLEEP_FOREVER,ADC_OFF,BOD_OFF,TIMER2_ON);  //108.47 ish on the USB C APM 

    //LowPower.powerStandby(SLEEP_FOREVER,ADC_OFF,BOD_OFF);  ///558.6 ish on the USB C APM   Takes 16 us to 32 us (depends on which point of the RC curve you measure from) to wake up 
    //LowPower.powerStandby(SLEEP_FOREVER,ADC_OFF,BOD_ON);  ///560.6 ish on the USB C APM     

    //LowPower.powerExtStandby(SLEEP_8S, ADC_OFF, BOD_OFF, TIMER2_OFF); //567.2.  Takes 16 us to 32 us (depends on which point of the RC curve you measure from) to wake up 

    // Disable analog comparator and it's interrupts.  Does not matter for power extStandby
    //16.65 vs 16.50 (after lots of waiting for R/C_ ... not perfect...) so we DO want to turn this off.. I really cant tell if this makes a difference but the datasheet says so.  Google Gemini claims that it draws 50uA but I can't see that via measurements.
    //ACSR = (ACSR & ~_BV(ACIE)) | _BV(ACD);
    //16.78 vs  vs 16.78 ... so I can't confirm this one...

    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //16.8 for USBC, 19mA for old APM, when powered via VCC.
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_ON, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //leaving timer0 on will basically mean that we don't sleep for longer than 1ms, ever.  BOO.
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //About 17.1 mA
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_ON, TIMER3_ON, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //about 17.3 uA
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //16.8 for USBC, 19mA for old APM, when powered via VCC.


    //BEWARE: Keeping USB_ON generates lots of interrupts that can easily, easily, throw things off here.


    //Use the following setting to keep Timer1 alive, so we can measure the duration of the pulse in.
    if(MEASURE_PULSEIN_DURATION)
    {
      LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON); //16.8 for USBC, 19mA for old APM, when powered via VCC.
    }
    //LowPower.adcNoiseReduction(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF);  //17.65; more than Idle (nteresting, I thought it would have been lower...)



    //NOTE:
    //TEST CONDITIONS/

    //Run this test with the USB cable unplugged so power only flows from the power supply.
    //ALSO, run this test AFTER A POWER-CYCLE.  The BOD_on vs BOD_OFF stuff was shown explicitly to have different behaviour if it wasn't vs was done after a power cycle (not just a reprogram) - THere may be other stuff too.

    //only do this when an external interrupt breaks me out of IDLE early.

  
}

void foo() //used
{
  //just an empty interrupt function, but we need an interrupt function to rbeak out of the low power modes when using TimerOne to kick us out of them.
}


void precise_idle(long tim) //works as long as A) you don't do multiplication in the argument and B) 
{
  Timer1.initialize(tim);
  Timer1.attachInterrupt(foo); //have to attach an interrupt to a blank function, otherwise, it wont generate an interrupt that will get us out of idle.
  LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_ON, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //About 17.1 mA
  Timer1.stop();  
}

//This CANNOT be blank, if so, the compiler optimizes it away.  I think.
void push() //triggered by LOW on the function generator (after the blink)
{
  if(digitalRead(interrupt_in)) //HIGH
  {
    if(MEASURE_PULSEIN_DURATION)
    {
      Timer1.restart(); //Start the the timer - so we start imediately when to start measuring.  We are measuring RISING Interrupt to FALLING interrupt (blue HIGH to blue LOW)
    }

    if(USE_LEDS)
    {
      digitalWrite(LED_BUILTIN_TX, LOW); // Turn RX LED on 
    } 
    
    //delay(250); //
    /*
    //IMPORTANT
    //If using IDLE with timer 0 off.  Or if using this in an interrupt
    //I cannot use delay in the interrupt (makes sense, the timers are off & they won't get turned on untill we're out of interrupt context - since they're turned on as the last step of the pwoer-down command (we interrupted in the middle of it)!)
    //have to use delayMicroseconds + for loops.
    */
    for(int i = 0; i < 1; i++) //approxiately 0.016 seconds.  ISH.
    {
      delayMicroseconds(16383); //special number, this is 14 bit max.
    }

    if(USE_LEDS)
    {
      digitalWrite(LED_BUILTIN_TX, HIGH); // Turn RX LED off
    }

  }

  else
  {
    if(MEASURE_PULSEIN_DURATION)
    {
      event_time = TCNT1; //save the time on Timer1 as of the time I fired this push.  DO it furst, so we can blink LEDS later.
    }
      //Uncomment to try the Idle self-waking
      //Timer1.stop(); //very first thing we do, 
      
      digitalWrite(mosfet,HIGH);
      if(USE_LEDS)
      {
        digitalWrite(LED_BUILTIN_RX, LOW); // Turn RX LED on
      }  
      
      //delay(250); //
      /*
      //IMPORTANT
      //If using IDLE with timer 0 off.  Or if using this in an interrupt
      //I cannot use delay in the interrupt (makes sense, the timers are off & they won't get turned on untill we're out of interrupt context - since they're turned on as the last step of the pwoer-down command (we interrupted in the middle of it)!)
      //have to use delayMicroseconds + for loops.
      */
      for(int i = 0; i < 20; i++) //approxiately 0.3 seconds.  ISH.
      {
        delayMicroseconds(16383); //special number, this is 14 bit max.
      }
      digitalWrite(mosfet,LOW);
      
      if(USE_LEDS)
      {
        digitalWrite(LED_BUILTIN_RX, HIGH); // Turn RX LED off
      }
    
      fired_push = true; //remember that we woke via PUSH, not some other internal interrupt.
    
  }

}
