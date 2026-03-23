
int n = 0;
int lastState = LOW;

void setup() {
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, OUTPUT);
  pinMode(9, INPUT);
  lastState = digitalRead(5);
}

// void loop() {
//   digitalWrite(8, LOW);
//   if(digitalRead(5) == HIGH || digitalRead(6) == HIGH || digitalRead(7) == HIGH || digitalRead(9) == HIGH){
//     digitalWrite(8, HIGH);
//   };       
// }

void loop() {
  int currentState = digitalRead(5);
  if (currentState == HIGH && lastState == LOW) {
    Serial.println("rising edge detected");
    n++;
  }
  lastState = currentState;

  if(n == 6){
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
    n = 0;
  }       
}

