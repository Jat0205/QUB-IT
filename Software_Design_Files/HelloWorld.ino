void setup() {
  pinMode(5, INPUT); //Jessica was here! 
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, OUTPUT);
}

void loop() {
  digitalWrite(8, LOW);
  if(digitalRead(5, HIGH)){
    digitalWrite(8, HIGH);
  };
            
}
//comment added by THOMAS for the Intro to GitHub assignment
//Jannet Trabelsi edit
