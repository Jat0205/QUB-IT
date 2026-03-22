const int LED_0 = 3;
const int LED_1 = 5;
const int LED_2 = 6;
const int LED_3 = 9;
const int LED_4 = 10;
const int LED_5 = 11;

int dim_value;
void setup(){
  Serial.begin(9600);
  dim_value = 255;
  pinMode(LED_0, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  pinMode(LED_5, OUTPUT);
}

void loop(){
  if(dim_value > 25){
    dim_value -= 25;
    delay(1000);}
  else if (dim_value < 25){
      dim_value = 0;
      delay(1000);
      dim_value = 255;

  }
  Serial.println(dim_value);
  analogWrite(LED_0, dim_value);
  analogWrite(LED_1, dim_value);
  analogWrite(LED_2, dim_value);
  analogWrite(LED_3, dim_value);
  analogWrite(LED_4, dim_value);
  analogWrite(LED_5, dim_value);
}