void setup_BEMF_sensing()
{
  pinMode(a_peak_rst,OUTPUT);
  pinMode(a_peak,INPUT);
  digitalWrite(a_peak_rst,HIGH); // don't reset the A_peak_sensing

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
  digitalWrite(a_peak_rst,LOW);
  delayMicroseconds(5);
  digitalWrite(a_peak_rst,HIGH);
}
