#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include "DFRobotDFPlayerMini.h"
#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))   // Using a soft serial port
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/4, /*tx =*/5);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

DFRobotDFPlayerMini myDFPlayer;

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20 column and 4 rows

// ============= GAME VARIABLES =============
bool gameRunning = true;
int score = 0;             //tracks player score
int currentCommand = 0;    //what player must do (X, Y, etc)
bool initialized = false;  //whether game has started
int qubitState = 0; //current state of the qubit


enum GamePhase { WAITING, INITIALIZE, GATE, MEASURE };
GamePhase currentPhase = INITIALIZE;
int expectedGate = 0;
bool waitingForNext = false;
bool processingCommand = false;  // Prevent double execution

unsigned long roundStartTime = 0;
unsigned long timeLimit = 3000;

int previousState = 0;  //stores old qubit state

// ============= QUBIT STATE ================
int lastStateX = LOW;  //used for edge detection for X encoder
int lastStateY = LOW;  //used for edge detection for Y encoder
int lastStateZ = LOW;  //used for edge detection for Z encoder
String COM = "";
int x, y, z = 0;
const int NUM_OPS = 8;
const int NUM_STATES = 6;
const int state_transition[NUM_OPS][NUM_STATES] = {
 {0, 0, 0, 0, 0, 0},  // INIT: always |0>
 {1, 0, 2, 3, 5, 4},  // X: swap 0↔1, swap 4↔5, keep 2,3
 {1, 0, 3, 2, 4, 5},  // Y: swap 0↔1, swap 2↔3, keep 4,5
 {0, 1, 3, 2, 5, 4},  // Z: keep 0,1, swap 2↔3, swap 4↔5
 {2, 3, 0, 1, 5, 4},  // H: swap 0↔2, swap 1↔3, swap 4↔5
 {-1, -1, 2, 3, -1, -1},  // XMEAS: every -1 needs sep. function to collapse to 2 or 3
 {-2, -2, -2, -2, 4, 5},  // YMEAS: every -2 needs sep. function to collapse to 4 or 5
 {0, 1, -3, -3, -3, -3},  // ZMEAS: every -3 needs sep. function to collapse to 1 or 0
};


bool LEDS[NUM_STATES] = {false, false, false, false, false, false};
bool INIT_LED = false; //LED on the controller that flashes whn turned on and held on when qubit is initialized
int coin = 0;


// ================= PINS ===================
const int ROTENCX = 2;
const int ROTENCY = 0;
const int ROTENCZ = 7;
const int DEBUGLED = 8;
const int PHOTODIODE = A3;
const int MEASUREX = A2;
const int MEASUREY = A1;
const int MEASUREZ = A0;
const int LEDpZ = 3;
const int LEDmZ = 9;
const int LEDpX = 1;
const int LEDmX = 6;
const int LEDpY = 10;
const int LEDmY = 11;
int CURRENTSTATE = LEDpZ;
int dim_value = 255;
bool LED_ON = false;

// ================== TIME ==================
float time_unit = 1000; // in ms
float duration = 5 + (100 * time_unit);
int wait_duration = 0;

// ================= SETUP =================
void setup() {
  // LCD
  lcd.init();
  lcd.backlight();

  // DFPlayer (D0/D1)
  FPSerial.begin(9600);
  #if (defined ESP32)
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/D3, /*tx =*/D2);
#else
  FPSerial.begin(9600);
#endif

  //Serial.begin(115200);
  
  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    
    while(true);
  }  
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  
  //----Set volume----
  myDFPlayer.volume(30);  //Set volume value (0~30).
  
  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  // Setting Pins
  pinMode(ROTENCX, INPUT);
  pinMode(ROTENCY, INPUT);
  pinMode(ROTENCZ, INPUT);
  pinMode(DEBUGLED, OUTPUT);
  pinMode(MEASUREX, INPUT);
  pinMode(MEASUREY, INPUT);
  pinMode(MEASUREZ, INPUT);
  pinMode(LEDpZ, OUTPUT);
  pinMode(LEDmZ, OUTPUT);
  pinMode(LEDpX, OUTPUT);
  pinMode(LEDmX, OUTPUT);
  pinMode(LEDpY, OUTPUT);
  pinMode(LEDmY, OUTPUT);
  analogWrite(LEDpZ, 0);
  analogWrite(LEDmZ, 0);
  digitalWrite(LEDpX, LOW);
  analogWrite(LEDmX, 0);
  analogWrite(LEDpY, 0);
  analogWrite(LEDmY, 0);

  lastStateX = digitalRead(ROTENCX);
  lastStateY = digitalRead(ROTENCY);
  lastStateZ = digitalRead(ROTENCZ);
  randomSeed(analogRead(A5));
}

// ================= INITIALIZE IT =================
void INITIALIZE_IT() {
  COM = "INITIALIZE IT!";
  lcd.clear();
  lcd.print(COM);
  SPEAKER(COM);
  delay(1000);
  wait_duration = 0;
  
  if (!initialized) {
    DETECT_PHOTODIODE();
  }
}

// ================= MEASURE IT =================
void MEASURE_IT() {
  COM = "MEASURE IT!";
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(COM);
  SPEAKER(COM);
  wait_duration = 0;
  
  bool result;
  int button_detected = 0;
  result = DETECT_MEASURE(button_detected);
  if (result == true){

  if(button_detected == MEASUREX){
    qubitState = state_transition[5][qubitState];
    qubitState = collapse_it(qubitState);
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
  }
  else if (button_detected == MEASUREY){
    int measureY = digitalRead(A1);
  if(measureY == HIGH){
    qubitState = state_transition[6][qubitState];
    qubitState = collapse_it(qubitState);
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
  }
  }
  else if (button_detected == MEASUREZ){
    int measureZ = digitalRead(A0);
  if(measureZ == HIGH){
    qubitState = state_transition[7][qubitState];
    qubitState = collapse_it(qubitState);
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
  }
  }
  diode_output(qubitState);
  recordSuccess();
  }
  else{
    recordFailure("4");
  }
}

// ================= GATE IT =================
void GATE_IT(String COM, int ENCODER_VAL) {
  lcd.setCursor(0, 0);
  lcd.print(COM);
  SPEAKER(COM);
  bool result;
  result = DETECT_GATE(ENCODER_VAL);
  if (result == true) {
    diode_output(qubitState);
    recordSuccess();
  } else {
    recordFailure("5");
  }
}

// ================= RECORD FAILURE =================
void recordFailure(String reason) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
  lcd.print(reason);
  
  gameRunning = false;
  while(true);
}

// ================= RECORD SUCCESS =================
void recordSuccess() {
  if (duration > time_unit) {
    duration -= time_unit;
  }
  score++;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GOOD!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
  delay(500);
  score_check();
  waitingForNext = false;
  generate_random_command();
}

// ================= SCORE CHECK =================
void score_check() {
  if (score >= 99) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("VICTORY!");
    lcd.setCursor(0, 1);
    lcd.print("Score: 99");
    lcd.setCursor(0, 2);
    lcd.print("You won!");
    
    gameRunning = false;
    while(true);
  }
}

// ================= SPEAKER =================
void SPEAKER(String COM) {
  if (COM == "INITIALIZE IT!") {
    myDFPlayer.play(7);
  } else if (COM == "X IT!") {
    myDFPlayer.play(10);
  } else if (COM == "Y IT!") {
    myDFPlayer.play(8);
  } else if (COM == "Z IT!") {
    myDFPlayer.play(4);
  } else if (COM == "MEASURE IT!") {
    myDFPlayer.play(2);
  } 
}

// ================= GENERATE RANDOM COMMAND =================
void generate_random_command() {
  currentCommand = random(0, 5);
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (currentCommand == 0) {
    currentPhase = INITIALIZE;
  } else if (currentCommand <= 3) {
    currentPhase = GATE;
    expectedGate = (currentCommand == 1) ? ROTENCX : 
                   (currentCommand == 2) ? ROTENCY : ROTENCZ;
  } else {
    currentPhase = MEASURE;
  }
  waitingForNext = true;
}

void detect_rotation(){
  int currentStateX = digitalRead(2);
  int currentStateY = digitalRead(0);
  int currentStateZ = digitalRead(7);
  if (currentStateX == HIGH && lastStateX == LOW) {
    x++;
  }
  lastStateX = currentStateX;
  
  if (currentStateY == HIGH && lastStateY == LOW) {
    y++;
  }
  lastStateY = currentStateY;
  
  if (currentStateZ == HIGH && lastStateZ == LOW) {
    z++;
  }
  lastStateZ = currentStateZ;
}

// ================= DETECT GATE =================
bool DETECT_GATE(int ENCODER_VAL) {
  wait_duration = 0;
  int n = 0;
  int wrong = 0;
  int mx, my, mz;
  do {
    detect_rotation();
    if (ENCODER_VAL == ROTENCX){
      n = x;
      if (y>z){
        wrong = y;
      }
      else{
        wrong = z;
      }
    }
    else if (ENCODER_VAL == ROTENCY){
      n = y;
      if (x>z){
        wrong = x;
      }
      else{
        wrong = z;
      }
    }
    else if (ENCODER_VAL == ROTENCZ){
      n = z;
      if (y>x){
        wrong = y;
      }
      else{
        wrong = x;
      }
    }
    detect_mx_my_mz(mx, my, mz);
    int sensorValue = analogRead(PHOTODIODE);    
    if (mx == HIGH || my == HIGH || mz == HIGH || sensorValue >= 512 || wrong>5) {
      return false;
    }
    
    wait_duration += time_unit;
    delay(time_unit);
    DIM_LED();
    } while (n<6 && wait_duration < duration);

  if (ENCODER_VAL == ROTENCX){
    if(x >= 5){
    qubitState = state_transition[1][qubitState];
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
    x = 0;
    return true;
  } 
  else{
    return false;
  }
  }
  else if (ENCODER_VAL == ROTENCY){
    if(y >= 5){
    qubitState = state_transition[2][qubitState];
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
    y= 0;
    return true;
  }
  else{
    return false;
  }
  }
  else{
if(z >= 5){
    qubitState = state_transition[3][qubitState];
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
    z = 0;
    return true;
  }
  else{
    return false;
  }
  }
}

int collapse_it(int qubitState){
  coin = TCNT0 & 1;
  if(qubitState == -1){
    if(coin == 0){
      return 2;
    }
    else{
      return 3;
    }
  }
  if(qubitState == -2){
    if(coin == 0){
      return 4;
    }
    else{
      return 5;
    }
  }
  if(qubitState == -3){
    if(coin == 0){
      return 1;
    }
    else{
      return 0;
    }
  }
  return qubitState;
}

void diode_output(int qubitState){
  digitalWrite(3, LOW);
  digitalWrite(9, LOW);
  digitalWrite(1, LOW);
  digitalWrite(6, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  switch (qubitState){
    case 0:
      digitalWrite(3, HIGH);
      break;
    case 1:
      digitalWrite(9, HIGH);
      break;
    case 2:
      digitalWrite(1, HIGH);
      break;
    case 3:
      digitalWrite(6, HIGH);
      break;
    case 4:
      digitalWrite(10, HIGH);
      break;
    case 5:
      digitalWrite(11, HIGH);
      break;
  }
}

// ================= DETECT PHOTODIODE =================
bool DETECT_PHOTODIODE() {
  int sensorValue = analogRead(PHOTODIODE);
  int mx, my, mz;
  
  while (sensorValue < 512 && wait_duration < duration) {
    detect_mx_my_mz(mx, my, mz);
    /*int currentStateX = digitalRead(2);
    int currentStateY = digitalRead(0);
    int currentStateZ = digitalRead(7);
      int number_of_mistakes = 0;
      if (currentStateX == HIGH && lastStateX == LOW) {
        number_of_mistakes++;
        int encoder_mistake = digitalRead(values[i]);
      }
      lastStateX = currentStateX;
      if (number_of_mistakes==2){
        recordFailure("1");
      }*/
    
    
    if (mx == HIGH || my == HIGH || mz == HIGH) {
      recordFailure("2");
    }
    
    wait_duration += time_unit;
    delay(time_unit);
    DIM_LED();
    sensorValue = analogRead(PHOTODIODE);
    
  }
  
  if (sensorValue >= 512){
    initialized = true;
    if (LED_ON) {
      if (CURRENTSTATE == LEDpX){
      digitalWrite(LEDpX, LOW);
      LED_ON = false;
    }
    else{
      analogWrite(CURRENTSTATE, 0);
      LED_ON = false;}
    }
    CURRENTSTATE = LEDpZ;
    analogWrite(CURRENTSTATE, 255);
    LED_ON = true;
    recordSuccess();
  }
  else{
      recordFailure("3");
    }
}

// ================= DETECT MX MY MZ =================
void detect_mx_my_mz(int &mx, int &my, int &mz) {
  mx = digitalRead(MEASUREX);
  my = digitalRead(MEASUREY);
  mz = digitalRead(MEASUREZ);
}

// ================= DETECT MEASURE =================
bool DETECT_MEASURE(int &button_detected) {
  wait_duration = 0;  // Reset wait_duration
  int mx, my, mz;
  
  detect_mx_my_mz(mx, my, mz);
  
  while (wait_duration < duration && mx == LOW && my == LOW && mz == LOW) {
    int sensorValue = analogRead(PHOTODIODE);
    if (sensorValue >= 512) {
      return false;
    }
    wait_duration += time_unit;
    delay(time_unit);
    DIM_LED();
    
    detect_mx_my_mz(mx, my, mz);
    
  }
  if (mx == HIGH || my == HIGH || mz == HIGH){
    
    if (mx == HIGH) {
      button_detected = MEASUREX;
    } else if (my == HIGH) {
      button_detected = MEASUREY;
    } else {
      button_detected = MEASUREZ;
    }
    return true;
  }
  else{
    return false;
}}

// ================= DIM LED =================
void DIM_LED() {
  if (LED_ON) {
    if (dim_value > 25) {
      if (CURRENTSTATE != LEDpX){
        dim_value -= 25;
      analogWrite(CURRENTSTATE, dim_value);
    }
    } else { 
    if (CURRENTSTATE != LEDpX){
      dim_value = 0;
      analogWrite(CURRENTSTATE, dim_value);
      LED_ON = false;
    }
    else{
      digitalWrite(CURRENTSTATE, LOW);
      LED_ON = false;}
    }
  }}

// ================= MAIN LOOP =================
void loop() {
  if (!gameRunning) return;
  
  switch(currentPhase) {
    case INITIALIZE:
      if (!initialized && !processingCommand) {
        processingCommand = true;
        INITIALIZE_IT();
        processingCommand = false;
      }
      break;
      
    case GATE:
      if (!processingCommand) {
        processingCommand = true;
        if (expectedGate == ROTENCX) {
          GATE_IT("X IT!", ROTENCX);
        } else if (expectedGate == ROTENCY) {
          GATE_IT("Y IT!", ROTENCY);
        } else if (expectedGate == ROTENCZ) {
          GATE_IT("Z IT!", ROTENCZ);
        }
        processingCommand = false;
      }
      break;
      
    case MEASURE:
      if (!processingCommand) {
        processingCommand = true;
        MEASURE_IT();
        processingCommand = false;
      }
      break;
      
    case WAITING:
      // Do nothing, waiting for next command
      break;
  }
}
