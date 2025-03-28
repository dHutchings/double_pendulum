#include <SlowSoftI2CMaster.h>  //software implimentation of i2c with only c code, no assembler instructions
#include <SlowSoftWire.h> //wire-like wrapper for the above library.

//order is data,clock.
SlowSoftWire Wire = SlowSoftWire(SDA_line, SCL_line);

#define V2 0x60
#define V1 0x61

float voltage_add = 2.0;  //Shift the voltage curve measured from the coil of wire up by 2 volts
float voltage_threshold = 1.65;  //Set the threshold to generate an trigger for the interrupt in to 1.65 volts.  Therefore, the voltage cuve must be -.35V to change the comparator's output.

void setup_dac()
{
  Wire.begin();

  set_voltage(V1, voltage_add);
  set_voltage(V2, voltage_threshold);

}

void dac_setup_permanent() //not used by code.  However, added so that future development can avoid having to put communcation lines between arduino & DACs on a regular basis.  Tested to work.
{
  Wire.begin();

  set_voltage_eeprom(V1, voltage_add);
  set_voltage_eeprom(V2, voltage_threshold);

}

void dac_setup_lite()
{
  //don't need to do anything.  Hooray!
}

void set_voltage(int MCP4725_ADDR, float val)
{
  //first, need to figure out the 12-bit number that corrisponds to the float value..
  //Datasheet: v_out = (v_ref * dn) / 4096.
  //Solve for dn, v_ref = 5;
  //dn = CEIL(v_out / v_ref * 4096) - 1 .
  float target = constrain(val, 0.0, 5.0); //constrain the target value to be between 0 and 5v (5V = power to DAC & max output)

  int value = int(target / 5.0 * 4096); //int truncates the float.

  value = constrain(value, 0, 4095); //and then subtract 1 if neccessary b/c of the 5V limit.




  Wire.beginTransmission(MCP4725_ADDR);
  Wire.write(64);  // we're explicity NOT in fast mode here.    Using Fig 6-2 of datasheet

  Wire.write(value >> 4); //8 most significant bits
  Wire.write( (value & 15) << 4); //4 LSB.
  Wire.endTransmission();



}

void set_voltage_eeprom(int MCP4725_ADDR, float val)
{
  //first, need to figure out the 12-bit number that corrisponds to the float value..
  //Datasheet: v_out = (v_ref * dn) / 4096.
  //Solve for dn, v_ref = 5;
  //dn = CEIL(v_out / v_ref * 4096) - 1 .
  float target = constrain(val, 0.0, 5.0); //constrain the target value to be between 0 and 5v (5V = power to DAC & max output)

  int value = int(target / 5.0 * 4096); //int truncates the float.

  value = constrain(value, 0, 4095); //and then subtract 1 if neccessary b/c of the 5V limit.


  Wire.beginTransmission(MCP4725_ADDR);
  Wire.write(96); //Set c1 and C0 high, see fig 6-2

  Wire.write(value >> 4); //8 most significant bits
  Wire.write( (value & 15) << 4); //4 LSB.
  Wire.endTransmission();


}

void reset_voltage_eeprom() //helper function for clearing eeprom.  not expected to be called regularly.
{
  set_voltage_eeprom(V2, 0);
  set_voltage_eeprom(V1, 0);
}


