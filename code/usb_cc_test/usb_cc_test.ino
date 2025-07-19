int cc1 = A0;
int cc2 = A1;

//It's impossible for the APM to determine - by CC alone - if we are on battery or on are only plugged in via the programming USB C connector.
//Since in both cases both voltages on CC1 and CC2 are zero.
//Fortunately, this does not matter (to the user), since if we are plugged in the programming APM connector, only 5V will be powered.
//The 15V line will not be powered up; the magnet will not run; but all the rest of the logic will.
//No big deal.
enum power_source {
  BATTERY,
  HALF_AMP,
  ONE_HALF_AMP,
  THREE_AMP,
  ERR,
  UNKNOWN,
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  delay(1000);
  Serial.print("Hello World!");
}

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
