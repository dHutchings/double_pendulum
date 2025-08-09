#define USE_BJT true //original incantations of the 555 timer circuit, following figure 7-1 and the other canonical stuff, use a BJT.
//but I thought I could use the DIO cleverly - but i failed 
//I leave this option here for posterity.


int mosfet = 16;  //pin 10... for turning the MOSFET on / pushing the magnet. back when the MOSFET and timer were the same sign.  On newer revisions of the board, there is a TIEMR_RESET
int auto_restart = 7; //pin 7... hooked up to the 555 timer.

int pulse_duration_ms = 200; //the time, in ms, that the pulse will be low / active / 'pushing'

int duration_between_pulses = 200; //this will incriment, from 200 up to 10,000, in 200 steps.


bool mos_state = false;

long last_changed_mos = 0;
long last_printed = 0;
long last_pulse_low_at = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println("Auto-Restart 555 Test!");
  delay(1000);

  
  #if USE_BJT
  pinMode(mosfet,OUTPUT);
  digitalWrite(mosfet,HIGH); //the normal, "not-pushing state"
  #else
  pinMode(mosfet,INPUT); //let N$7 float.
  #endif
  
  pinMode(auto_restart,INPUT);

  last_changed_mos = millis();
  last_printed = millis();
}

void loop() {
  if(! mos_state & (millis() - last_pulse_low_at > pulse_duration_ms))
  {
    mos_state = true;
    #if USE_BJT    
    digitalWrite(mosfet,mos_state);
    #else
    pinMode(mosfet,OUTPUT);
    digitalWrite(mosfet,LOW); //directly discharge N$7 through the DIO.
    #endif
    
    last_changed_mos = millis();
    duration_between_pulses += 200; //the next pulse will be less often than the curent one.
    if(duration_between_pulses > 10000)
    {
      duration_between_pulses = 200;
    }
  }
  
  // put your main code here, to run repeatedly:
  if(mos_state & ( millis() - last_changed_mos > duration_between_pulses))
  {
    mos_state = false;
    #if USE_BJT    
    digitalWrite(mosfet,mos_state);
    #else
    pinMode(mosfet,INPUT); //let N$7 float again
    #endif
    last_changed_mos = millis();
    last_pulse_low_at = millis();
  }

  if(millis() - last_printed > 20) 
  {
    //the arduino serial monitor's window size is 500 points, so printing at 20ms gives a 10 second window
    //20 ms period is too fast for the USB timestamps to be accurate, but, the serial plotter doesn't care since it plots by data point, not time recieved
    Serial.print("MOS:");
    Serial.print(int(mos_state));
    Serial.print(",555:");
    Serial.print(digitalRead(auto_restart));
    Serial.print(",Dur:");
    Serial.println(float(duration_between_pulses)/float(1000));
    last_printed = millis();
  }
  
  delay(1); //allow loop to run every ms.  

}
