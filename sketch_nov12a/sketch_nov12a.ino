#include <LowPower.h>



int sense = A10;
int coil_drive = 9;  //remember that it's inverted due to PMOS.  LOW to drive, HIGH to not drive.

int sensorValue;

int normal_value = 530; //nominal "normal".. ie, no siginal value.
int threshold = 5; //counts over "normal" value.


unsigned long push_time = 13; //push for 20ms past the start of the peak.

bool push; //if i'm pushing!

bool prints = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //have to let it sense the analog read when the MOSFET ain't doing nothing... when it's set to an input.
  pinMode(coil_drive,INPUT);
  delay(50);
  normal_value = analogRead(sense);
  
  pinMode(coil_drive,OUTPUT); //only make it an ouput when i'm pushing.
  digitalWrite(coil_drive,HIGH); //set it off for the time being.
  delay(20); //let it take effect.

  
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorValue = analogRead(sense);

  if(sensorValue > (normal_value + threshold))
  {
    if(prints)
    {
      Serial.println("PUSH");
      Serial.print("Sensor Value:");
      Serial.println(sensorValue);
      Serial.print("normal Value:");
      Serial.println(normal_value);
    }
    
    //digitalWrite(led,HIGH);
    digitalWrite(coil_drive,LOW);
    //last_light = millis();
    

    delay(push_time);

    digitalWrite(coil_drive,HIGH);

    delay(2*push_time); //allow a reset.  seems to be that the RC curve takes this long to die down.
 
  }
  
  if(prints)
  {
    int diff = sensorValue - normal_value;
    Serial.println(diff);
  }
  
  delayMicroseconds(500);
}

void push_ms(int tim) //blocking push for a certian time.
{
    digitalWrite(coil_drive,LOW);
    delay(tim);
    digitalWrite(coil_drive,HIGH);

}
