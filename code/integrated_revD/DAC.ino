#include <SPI.h>
#include <LTC1661.h>

#include "constants.h"

LTC1661 dac(cs);    //creats an instance with chipSelect as CS Pin


//The op-amp takes time to swing, so, its best to trigger a little early before the zero-crossing.  This was dialed in via o-scope tests.  It looks like it takes approx about 1.8 ms to swing, hence, why we are triggering < 2v substantially.

int ch_a_rising = 0;
int ch_b_rising = 0;
int ch_a_falling = 0;
int ch_b_falling = 0;

void setup_dac()
{

  //no begin functionality with this library.
  //we just hope that it's soldered and the communications are working.

  //just calculate the bit values so we don't recompute them every time we have to communicate via SPI
  ch_a_rising = constrain(int(voltage_add/5.0*1024), 0, 1023);
  ch_b_rising = constrain(int(voltage_threshold_rising/5.0*1024), 0, 1023);

  ch_a_falling = constrain(int(voltage_add/5.0*1024), 0, 1023);
  ch_b_falling = constrain(int(voltage_threshold_falling/5.0*1024), 0, 1023);


}

void reset_daq_settings()
{
  dac.loUp(ch_a_falling,ch_b_falling);  //send values and update both OUTPUTS
  
}

void change_daq_settings()
{
    dac.loUp(ch_a_rising,ch_b_rising);  //send values and update both OUTPUTS

}
