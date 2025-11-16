#ifndef CONSTANTS_H
#define CONSTANTS_H


/* Kinematics Constants: PUSH TIME and RANDOMNESS */
static long NOMINAL_PUSH = 5500; //nominal push time, fixed value that we always use.
static long MAX_PUSH = 25000;
static long MIN_PUSH = 500;
static long PUSH_SETTING_STEP = 500;

volatile long push_time_us = NOMINAL_PUSH;  //actual push time.  It needs to be here, since multiple code segments deal with it.  Can be adjusted up and down via button presses

volatile long RANDOM_AMOUNT = 3000; //allow for +/- 3000uS push time randomness... used to prevent long-term cyclic oscilations.
volatile long random_time; //random value, can have a negative value so not unsigned.  This is actually the dyamically computed random value

#define SCALE_RANDOMNESS true //set to true to enable the amount of randomness to be scaled up and down by how much we are over or underpushing, not just some nominal value.
//the real random push time will be scaled up or down from this value.  EG the nominal push time is 5500uS.  If it's half of that (2750 uS), the nominal random time max could cause a negative push time.


volatile int MAX_PUSHES = 3; //number of times the pendulum will push untill is chooses a new random push time
volatile int CHANCE_NO_PUSH = 10; //5% chance of not pushing this time, b/c randomness.  Set to -1 to turn off.


volatile float last_bemf; //the value of the most recent BEMF


/* DEBUG Modes */
#define DEBUG false //set to true for debug-only TX / RX LEDs.  TX LED (left?) is generally around the drive interrupt trigger times, RX is around the MOSFET driving.
#define DEBUG_PRINTS false //set to tue for debug-only prints.  This means the system cannot power down (USB issues).  ALSO, be aware that if the serial monitor window isnt open but we are still trying to send prints, the pendulum will stop working, too.
#define NO_PUSH false //set to true for absolutely no pushing or driving on the COIL wire.
#define MOD_DEBUG_PRINTS false //wakeup-mode prints.  These things fill the screen
/* Curve Threshold Sensing Offsets */
float voltage_add = 1.2;  //Shift the voltage curve measured from the coil of wire up by 2 volts.  We set the V1 to be 1.2V and the op-amp resistor feedback network will scale the COIL and shift it up by 2.
float voltage_threshold = 1.65;  //Set the threshold to generate an trigger for the interrupt in to 1.65 volts.  Therefore, the voltage cuve must be -.35V to change the comparator's output.  The interrumpt will be generated low-to-high just before the zero point.


#endif
