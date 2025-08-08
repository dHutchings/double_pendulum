#include <FlexWire.h> //Drop-in library that overrides the wire lib to use bit-banging on any pins.
#include <Adafruit_MCP4728.h>

//order is data,clock.
FlexWire Wire = FlexWire(SDA_line, SCL_line);

Adafruit_MCP4728 mcp;

float voltage_add = 1.2;  //Shift the voltage curve measured from the coil of wire up by 2 volts.  We set the V1 to be 1.2V and the op-amp resistor feedback network will scale the COIL and shift it up by 2.
float voltage_threshold = 1.65;  //Set the threshold to generate an trigger for the interrupt in to 1.65 volts.  Therefore, the voltage cuve must be -.35V to change the comparator's output.  The interrumpt will be generated low-to-high just before the zero point.

//The op-amp takes time to swing, so, its best to trigger a little early before the zero-crossing.  This was dialed in via o-scope tests.  It looks like it takes approx about 1.8 ms to swing, hence, why we are triggering < 2v substantially.

void setup_dac()
{
  Wire.begin();

  if (!mcp.begin()) {
    Serial.println("Failed to find MCP4728 chip");
    while (1) {
      delay(10);
    }
  }

  //set channel A and B on the DAC (V1 / V2, respectively) to their correct values.
  //using the internal Vref (2.048V), which does consume a bit of extra power (60 uA) but it's more stable.
  mcp.setChannelValue(MCP4728_CHANNEL_A, constrain(int(voltage_add/2.048*4096), 0, 4095) ,MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);
  mcp.setChannelValue(MCP4728_CHANNEL_B, constrain(int(voltage_threshold/2.048*4096), 0, 4095), MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X);

  //Turn the other channels off, according to the datasheet this saves 400 uA and is the only reason this chip consumes less power than the older 1 channel DACs
  mcp.setChannelValue(MCP4728_CHANNEL_C, 0,MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X,MCP4728_PD_MODE_GND_500K);
  mcp.setChannelValue(MCP4728_CHANNEL_D, 0,MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X,MCP4728_PD_MODE_GND_500K);

}
