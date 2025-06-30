// Basic demo for configuring the MCP4728 4-Channel 12-bit I2C DAC
#include "Wire.h"


int SCL_line = 8;
int SDA_line = 6;
FlexWire Wire = FlexWire(SDA_line, SCL_line);

#include <Adafruit_MCP4728.h>

Adafruit_MCP4728 mcp = Adafruit_MCP4728();

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MCP4728 test!");

  // Try to initialize!
  if (!mcp.begin()) {
    Serial.println("Failed to find MCP4728 chip");
    while (1) {
      delay(10);
    }
  }

  mcp.setChannelValue(MCP4728_CHANNEL_A, 4095);  //using external VREF, maximum value
  mcp.setChannelValue(MCP4728_CHANNEL_B, 2048, MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X); //using the internal Vref (2.048V) set to 1/2 of the full 12-bit value, so should be 1.024V.

  //Turn the other channels off, according to the datasheet this saves 400 uA and is the only reason this chip consumes less power than the older 1 channel DACs
  mcp.setChannelValue(MCP4728_CHANNEL_C, 0,MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X,MCP4728_PD_MODE_GND_500K);
  mcp.setChannelValue(MCP4728_CHANNEL_D, 0,MCP4728_VREF_INTERNAL, MCP4728_GAIN_1X,MCP4728_PD_MODE_GND_500K);
}

void loop() { delay(1000); }
