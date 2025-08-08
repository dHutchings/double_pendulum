#include <LowPower.h>

#include <TimerOne.h> //use internal timer for precise wake-ups (only when in idle).  See quickstart 57.  It only works for periods < 8.3 seconds.


//code that will test power consumption of CPU under various conditions.

int mosfet = 9;  //pin 9
int interrupt_in = 7;  //pin 3.  Was pin 7, but pin 7 maps to interupt #4, which doesn't have the ability to wake from power off.

volatile unsigned long push_time_us = 12*1000;
 

void setup() {
  // put your setup code here, to run once:


  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,LOW);

  
  pinMode(interrupt_in,INPUT); //hook up to function generator.
  //But be careful, can only wake up on changes to interupts 0:3, pins 3,2,1,0.

  
  //attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);

  pinMode(LED_BUILTIN_RX, OUTPUT); // TX LED
  pinMode(LED_BUILTIN_TX, OUTPUT); // TX LED
  digitalWrite(LED_BUILTIN_RX, LOW); // Turn TX LED on
  digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on
  delay(5000); //time for the uC to be booted, making it easier to reprogram

  digitalWrite(LED_BUILTIN_RX, HIGH); // Turn TX LED off  
  digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off


  //Uncomment to try the Idle self-waking
  /*
  Timer1.attachInterrupt(push);
  Timer1.initialize(8388480 ); //8388480  is from https://deepbluembedded.com/arduino-timerone-library/#:~:text=Timer1.setPeriod(period)&text=The%20minimum%20period%20or%20highest,frequencies%20and%20duty%20cycles%20simultaneously.
  */

  


}

void loop() {

    //I'm in loop once We're done with the interrupt.
    //Twiddle some lights
    //And then immediately go back to sleep
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.

    detachInterrupt(digitalPinToInterrupt(interrupt_in)); //detach interrupt immediately, as a form of debounce protection

    digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on  
    delay(1500);
    digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off  
    delay(1500);

    attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING); //reattach interrupt so I can wake back up
    //pin 0,1,2,3 can do CHANGE

    //31mA when no power saving is applied.
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //16.8 for USBC, 19mA for old APM, when powered via VCC.

    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_ON, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //leaving timer0 on will basically mean that we don't sleep for longer than 1ms, ever.  BOO.

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
    
    LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF); //73.72 uA on the USB C APM.  118 uA on the old APM.  but only on pins 2&3.  0&1 draw ~.5mA... and on the new APM there isn't any pin 1 vs pin 3 performance difference.  Takes 3.65 ms to wake up.
    //For some reason, BOD_ON results in less power dissipation.  Wierd.



    //LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //108.15 on the USB C APM  227 uA on the Old APM.  But be careful, can only wake up on changes to interupts 0:3, pins 3,2,1,0.  

    //LowPower.powerSave(SLEEP_FOREVER,ADC_OFF,BOD_OFF,TIMER2_OFF);  //108.35 ish on the USB C APM  Takes 3.63 ms to wake up.
    //LowPower.powerSave(SLEEP_FOREVER,ADC_OFF,BOD_OFF,TIMER2_ON);  //108.47 ish on the USB C APM 

    //LowPower.powerStandby(SLEEP_FOREVER,ADC_OFF,BOD_OFF);  ///558.6 ish on the USB C APM   Takes 16 us to 32 us (depends on which point of the RC curve you measure from) to wake up 
    //LowPower.powerStandby(SLEEP_FOREVER,ADC_OFF,BOD_ON);  ///560.6 ish on the USB C APM     

    //LowPower.powerExtStandby(SLEEP_8S, ADC_OFF, BOD_OFF, TIMER2_OFF); //567.2.  Takes 16 us to 32 us (depends on which point of the RC curve you measure from) to wake up 




    //NOTE:
    //TEST CONDITIONS/

    //Run this test with the USB cable unplugged so power only flows from the power supply.
    //ALSO, run this test AFTER A POWER-CYCLE.  The BOD_on vs BOD_OFF stuff was shown explicitly to have different behaviour if it wasn't vs was done after a power cycle (not just a reprogram) - THere may be other stuff too.

  
}

void push() //used
{
  //Uncomment to try the Idle self-waking
  //Timer1.stop(); //very first thing we do, 
  
  digitalWrite(mosfet,HIGH);
  delayMicroseconds(100);
  digitalWrite(mosfet,LOW);
  digitalWrite(LED_BUILTIN_RX, LOW); // Turn RX LED on  
  
  delay(250);
  //Uncomment to try the Idle self-waking
  /*
  //IMPORTANT
  //If using IDLE with timer 0 off
  //I cannot use delay in the interrupt (makes sense, the timers are off!)
  //have to use delayMicroseconds + for loops.
  for(int i = 0; i < 20; i++)
  {
    delayMicroseconds(16383);
  }
  */

  digitalWrite(LED_BUILTIN_RX, HIGH); // Turn RX LED on  

}
