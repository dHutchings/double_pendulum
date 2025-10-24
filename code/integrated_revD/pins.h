#ifndef PINS_H
#define PINS_H

//PinDefs.

//the most important pins, related to sensing zero crossing + pushing
int interrupt_in = 3;  //pin 3, which can trigger an interrupt for waking up from power down.
int drive_mosfet = 8;  //pin 10... for turning the MOSFET on / pushing the magnet.

//pins for automatic missing pulse restarting.
int auto_timer_restart = 7; //pin 7... hooked up to the 555 timer - used for measuring when the 555 timer says its time to restart
int timer_rst = 16; //pin 16... hooked up to the 555 timer - used for clearing the timer.

//pins for measuring USB power capability
int cc1 = A0;
int cc2 = A1;

//pins for measuring the peak voltages
int a_peak = A6; //pin 4, but, for reasons surpassing understanding you do really need to say A6 //https://forum.arduino.cc/t/analogread-returns-varying-values-when-connected-to-gnd-3-3v-5v/460706
int a_peak_rst = 5;

//i2c capabilities
int SCL_line = 9;
int SDA_line = 6;//clock is pin 7, data is pin 6.

//UI buttons
int faster = 2;  //pin 2, Switch S3 (left)
int slower = 1;  //pin 1, Switch S1 (right)
int start = 0;  //pin 0, Switch S2 (middle)












#endif
