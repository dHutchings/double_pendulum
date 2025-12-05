#include <SPI.h>
#include <LTC1661.h>

#include "constants.h"

LTC1661 dac(cs);    //creats an instance with chipSelect as CS Pin


//The op-amp takes time to swing, so, its best to trigger a little early before the zero-crossing.  This was dialed in via o-scope tests.  It looks like it takes approx about 1.8 ms to swing, hence, why we are triggering < 2v substantially.

void setup_dac()
{

  //no begin functionality with this library.
  //we just hope that it's soldered and the communications are working.

  
  dac.loUp(constrain(int(voltage_add/5.0*1024), 0, 1023), constrain(int(voltage_threshold/5.0*1024), 0, 1023));  //send values and update both OUTPUTS



}
