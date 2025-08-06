int drive_mosfet = 8;  //pin 10 (rev C), pin 8 (rev D) ... for turning the MOSFET on / pushing the magnet.


void setup() {
  // put your setup code here, to run once:
  pinMode(drive_mosfet,OUTPUT);
  digitalWrite(drive_mosfet,LOW);

  pinMode(LED_BUILTIN_RX, OUTPUT); // TX LED
  pinMode(LED_BUILTIN_TX, OUTPUT); // TX LED
  digitalWrite(LED_BUILTIN_RX, LOW); // Turn TX LED off  
  digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED off
   
}




void loop() {
  // put your main code here, to run repeatedly:
    digitalWrite(drive_mosfet,HIGH);
    digitalWrite(LED_BUILTIN_RX, LOW); // Turn TX LED on  
    digitalWrite(LED_BUILTIN_TX, LOW); // Turn TX LED on


    delay(5000);
    digitalWrite(drive_mosfet,LOW);
    digitalWrite(LED_BUILTIN_RX, HIGH); // Turn TX LED off  
    digitalWrite(LED_BUILTIN_TX, HIGH); // Turn TX LED off

    delay(10000);


}
