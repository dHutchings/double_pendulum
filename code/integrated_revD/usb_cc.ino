enum power_source {
  BATTERY,
  HALF_AMP,
  ONE_HALF_AMP,
  THREE_AMP,
  ERR,
  UNKNOWN,
};


/*
*   //suggests that I should set
  /*
  DIDR0 |= _BV(ADC7D) | _BV(ADC6D);
  */
  /*
 *
 void loop() {
  // put your main code here, to run repeatedly:
  float v_cc1 = float(analogRead(cc1))*5/1024;
  float v_cc2 = float(analogRead(cc2))*5/1024;
 
  Serial.print(v_cc1); //convert from 10 bit to 0 to 5 V
  Serial.print("\t");
  Serial.print(v_cc2);

  float max_voltage = max(v_cc1,v_cc2);
  float min_voltage = min(v_cc1,v_cc2);

  int power_state = UNKNOWN;

  if( min_voltage < -0.1 || min_voltage > 0.1)
  {
    //some kind of error
    power_state = ERR;
    Serial.print("\t error");
  }
  else if(max_voltage < 0.1)
  {
    power_state = BATTERY;
    Serial.print("\t Battery");
  }
  else if(0.25 < max_voltage && max_voltage < 0.61)
  {
    power_state = HALF_AMP;
    Serial.print("\t 0.5 Amp");
  }
  else if(0.70 < max_voltage && max_voltage < 1.16)
  {
    power_state = ONE_HALF_AMP;
    Serial.print("\t 1.5 Amp");
  }
  else if(1.31 < max_voltage && max_voltage < 2.04)
  {
    power_state = THREE_AMP;
    Serial.print("\t Three Amp");
  }
  else
  {
    Serial.print("\t Unknown");
  }

  Serial.println();


  delay(100);

}
 */
