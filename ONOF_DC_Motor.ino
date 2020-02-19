
int motorPin = 13;
int counter = 10;
void setup() {
   pinMode(motorPin, OUTPUT);
   Serial.begin(9600);
}
void loop() {
   if (counter > 0) {
      Serial.print("counter: ");
      Serial.println(counter);
      digitalWrite(motorPin, HIGH);
      delay(10000);
      digitalWrite(motorPin, LOW);
      delay(86400000);
      counter -- ;
   }
   else{
      Serial.println("DON! ");
      counter = 10;
   }
}
