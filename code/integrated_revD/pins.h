#ifndef PINS_H
#define PINS_H

//PinDefs.

//the most important pins, related to sensing zero crossing + pushing
int interrupt_in = 3;  //pin 3, which can trigger an interrupt for waking up from power down.
int drive_mosfet = 8;  //pin 8... for turning the MOSFET on / pushing the magnet through the level shifter.

//pins for automatic missing pulse restarting.
int auto_timer_restart = 2; //pin 2... hooked up to the 555 timer - used for measuring when the 555 timer says its time to restart
int timer_rst = 4; //pin 9... hooked up to the 555 timer - used for clearing the timer.

//pins for measuring USB power capability
int cc1 = A0;
int cc2 = A1;

//pins for measuring the peak voltages
int a_peak = A7; //pin 6, but, for reasons surpassing understanding you do really need to say A7 //https://forum.arduino.cc/t/analogread-returns-varying-values-when-connected-to-gnd-3-3v-5v/460706
int a_peak_rst = 9;

//SPI capabilities
int cs = 10; 
//the rest of the pins are default: MISO on 14, MOSI on 16, SCK on 15.

//UI buttons
int faster = 0;  //pin 2, Switch S3 (right)
int slower = 1;  //pin 1, Switch S1 (left)
int start = 7;  //pin 0, Switch S2 (middle)












#endif
