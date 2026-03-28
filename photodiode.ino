int sensorValue;
void setup() {
  pinMode(8, OUTPUT);
}

void loop() {
  sensorValue = analogRead(A3);
  if (sensorValue >= 512)
  digitalWrite(8, HIGH); 
  delay(2);
}