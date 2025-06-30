#ifndef PINS_H
#define PINS_H

//PinDefs.
int interrupt_in = 3;  //pin 3, which can trigger an interrupt for waking up from power down.
int mosfet = 10;  //pin 10... for turning the MOSFET on / pushing the magnet.
int auto_restart = 7; //pin 7... hooked up to the 555 timer.


int SCL_line = 8;
int SDA_line = 6;//clock is pin 7, data is pin 6.

//UI buttons
int faster = 2;  //pin 2, Switch S3 (left)
int slower = 1;  //pin 1, Switch S1 (right)
int start = 0;  //pin 0, Switch S2 (middle)












#endif
