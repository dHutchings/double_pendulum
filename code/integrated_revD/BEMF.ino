void setup_BEMF_sensing()
{
  pinMode(a_peak_rst,INPUT); // dont reset the A_peak_sensing
  pinMode(a_peak,INPUT);
  DIDR2 |= _BV(ADC8D);
  //per the datasheet, 7.8.6. we need to explicitly disable the digital input buffer since we're using it as an analog port.
  //This change makes little difference at rest .. but it's thing to do according to the datasheet.

}

float measure_BEMF()
{
  int val = analogRead(a_peak); 

  float v_peak = float(val)*float(5000)/float(1024); //I like millivolts, not volts.
  return v_peak;
  //return 2000 - v_peak; //Probably, in final controls mode, I would like the sign reversed so larger is faster.  But right now, return the number I can confirm with o-scope
}

//This is all we need to reset the BEMF measurement circuit, but, it is incumbent on whoever calls this function to make sure it's a good time to reset it!
void clear_BEMF_sensing()
{
  pinMode(a_peak_rst,OUTPUT);
  digitalWrite(a_peak_rst,HIGH); //directly load 5V into the capacitor
  delayMicroseconds(5);
  pinMode(a_peak_rst,INPUT); //turn the DIO back into an input, so its floating and the PEAK_RST line is now controlled by the op=amps.
}
