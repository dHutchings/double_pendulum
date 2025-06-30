int mosfet = 10;  //pin 10... for turning the MOSFET on / pushing the magnet.
int auto_restart = 7; //pin 7... hooked up to the 555 timer.

bool mos_state = false;

long last_changed_mos = 0;
long last_printed = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Auto-Restart 555 Test!");
  delay(1000);

  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,HIGH); //the normal, "not-pushing state"
  pinMode(auto_restart,INPUT);

  last_changed_mos = millis();
  last_printed = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if( millis() - last_changed_mos > 2500)
  {
    mos_state = !mos_state;
    digitalWrite(mosfet,mos_state);
    last_changed_mos = millis();
  }

  if(millis() - last_printed > 20) 
  {
    //the arduino serial monitor's window size is 500 points, so printing at 20ms gives a 10 second window
    //20 ms period is too fast for the USB timestamps to be accurate, but, the serial plotter doesn't care since it plots by data point, not time recieved
    Serial.print("MOS:");
    Serial.print(int(mos_state));
    Serial.print(",555:");
    Serial.println(digitalRead(auto_restart));
    last_printed = millis();
  }
  
  delay(1); //allow loop to run every ms.  

}
