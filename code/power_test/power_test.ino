#include <LowPower.h>

//code that will test power consumption of CPU under various conditions.

int mosfet = 9;  //pin 9
int interrupt_in = 0;  //pin 3.  Was pin 7, but pin 7 maps to interupt #4, which doesn't have the ability to wake from power off.

volatile unsigned long push_time_us = 12*1000;

volatile int prev_interrupts;
 

void setup() {
  // put your setup code here, to run once:


  prev_interrupts = 0;

  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,!digitalRead(mosfet));

  delay(100);
  
  pinMode(interrupt_in,INPUT); //hook up to function generator.
  //attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING); //used to demonstrate that the interupt (and uC) is actually working).
  

  delay(100);


}

void loop() {
    // Allow wake up pin to trigger interrupt on low.
    //digitalWrite(is_awake,HIGH);
    attachInterrupt(digitalPinToInterrupt(interrupt_in),push,FALLING);
    
    // Enter power down state with ADC and BOD module disabled.
    // Wake up when wake up pin is low.

    //31mA when no power saving is applied.
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF); //19mA, when powered via VCC.
    
    LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_ON); //118 uA.  but only on pins 2&3.  0&1 draw ~.5mA.
    //But be careful, can only wake up on changes to interupts 0:3, pins 3,2,1,0.
    //For some reason, BOD_ON results in less power dissipation.  Wierd.
    

    //LowPower.powerDown(SLEEP_FOREVER,ADC_OFF,BOD_OFF); //227 uA.  But be careful, can only wake up on changes to interupts 0:3, pins 3,2,1,0.
    
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON); //23mA, when powered via VCC.
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, TIMER0_ON, SPI_OFF, USART1_OFF, TWI_OFF, USB_ON); //23.3 mA, when powered via VCC.




    //digitalWrite(is_awake,HIGH);
    // Disable external pin interrupt on wake up pin.
    detachInterrupt(digitalPinToInterrupt(interrupt_in));
    
    // Do something here
    // Example: Read sensor, data logging, data transmission.
  
}

void push() //used
{
  digitalWrite(mosfet,!digitalRead(mosfet));
 

}


