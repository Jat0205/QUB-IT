void setup() {
  pinMode(A2, INPUT); //measure x
  pinMode(A1, INPUT); //measure y
  pinMode(A0, INPUT); //measure z
  pinMode(8, OUTPUT); //output verification
}

void loop() {
  int measureX = digitalRead(A2);
  int measureY = digitalRead(A1);
  int measureZ = digitalRead(A0);
  if(measureX == HIGH || measureY == HIGH || measureZ == HIGH){
    digitalWrite(8, HIGH);
  }
  else{
    digitalWrite (8, LOW);
  }
}
