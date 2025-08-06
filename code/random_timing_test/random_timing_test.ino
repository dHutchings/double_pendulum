void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(8,OUTPUT);
  digitalWrite(8,LOW); //using nmos, pmos.  HIGH is on. LOW is off.

}

void loop() {
  // put your main code here, to run repeatedly:
  long t1 = micros();

  int num = random(0,100);
  long t2 = micros();
  Serial.print(t2-t1);
  Serial.print("\t");
  Serial.print(num);
  Serial.println();
  delay(500); 
}
