#include <LiquidCrystal_I2C.h>
#include "DFRobotDFPlayerMini.h"

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ================= DFPLAYER (USES D0/D1) =================
#define FPSerial Serial
DFRobotDFPlayerMini myDFPlayer;

// ================= GAME VARIABLES =================
int score = 0;             //tracks player score
int currentCommand = 0;    //what player must do (X, Y, etc)
bool initialized = false;  //whether game has started

unsigned long roundStartTime = 0;
unsigned long timeLimit = 3000;

int previousState = 0;  //stores old qubit state

// ================= QUBIT STATE =================
int qubitState = 0;  //stores current state

// ================= STATE TRANSITION =================
const int state_transition[8][6] = {
  { 0, 0, 0, 0, 0, 0 },
  { 1, 0, 2, 3, 5, 4 },
  { 1, 0, 3, 2, 4, 5 },
  { 0, 1, 3, 2, 5, 4 },
  { 2, 3, 0, 1, 5, 4 },
  { -1, -1, 2, 3, -1, -1 },
  { -2, -2, -2, -2, 4, 5 },
  { 0, 1, -3, -3, -3, -3 }
};

// =================GLOBALS =================
int x = 0;  //x,y,z counts encoder rotations
int y = 0;
int z = 0;
int lastStateX = LOW;  //used for edge detection
int lastStateY = LOW;
int lastStateZ = LOW;
int coin = 0;  //used for collapse

// ================= SETUP =================
void setup() {

  // LCD
  lcd.init();
  lcd.backlight();

  // DFPlayer (D0/D1)
  FPSerial.begin(9600);
  myDFPlayer.begin(FPSerial);
  myDFPlayer.volume(10);

  // Pins
  pinMode(2, INPUT);  //2,4,7 rotary encoders
  pinMode(4, INPUT);
  pinMode(7, INPUT);

  pinMode(A2, INPUT);  //measurement buttons
  pinMode(A1, INPUT);
  pinMode(A0, INPUT);

  pinMode(3, OUTPUT);  //LEDs
  pinMode(9, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);

  pinMode(A3, INPUT);  //photodiode

  lastStateX = digitalRead(2);  //initialize encoder states
  lastStateY = digitalRead(4);
  lastStateZ = digitalRead(7);

  lcd.setCursor(0, 0);
  lcd.print("INITIALIZE IT!");
}

// ================= MAIN LOOP =================
void loop() {

  // ---------- WAIT FOR INITIALIZATION ----------
  if (!initialized) {
    int sensorValue = analogRead(A3);

    if (sensorValue > 512) {
      initialized = true;
      score = 0;
      qubitState = 0;

      lcd.clear();
      lcd.print("START!");
      myDFPlayer.play(1);  // init sound
      delay(500);
    }
    return;
  }

  // ---------- START ROUND ----------
  currentCommand = random(1, 5);

  startRound(currentCommand);
  previousState = qubitState;

  bool success = false;

  // ---------- INPUT LOOP ----------
  while (!isTimeUp()) {
    //detects input, update qubit state
    detect_rotation();
    detect_Xgate();
    detect_Ygate();
    detect_Zgate();
    detect_Xmeasure();
    detect_Ymeasure();
    detect_Zmeasure();

    diode_output(qubitState);

    // detect change
    if (qubitState != previousState) {

      // ===== CHECK CORRECTNESS =====
      if (currentCommand == 1 && x == 0) success = true;
      else if (currentCommand == 2 && y == 0) success = true;
      else if (currentCommand == 3 && z == 0) success = true;
      else if (currentCommand == 4) success = true;
      else {
        recordFailure("WRONG INPUT");
        initialized = false;
        return;
      }

      break;
    }
  }

  // ---------- TIMEOUT ----------
  if (!success) {
    recordFailure("TIME UP");
    initialized = false;
    return;
  }

  // ---------- SUCCESS ----------
  recordSuccess();
  // decrease time for next round 
  if (timeLimit > 1000) {
    timeLimit -= 100;
  }
}

// ================= HELPER FUNCTIONS =================

void startRound(int command) {
  lcd.clear();
  lcd.setCursor(0, 0);

  if (command == 1) {
    lcd.print("X IT!");
    myDFPlayer.play(2);
  }
  if (command == 2) {
    lcd.print("Y IT!");
    myDFPlayer.play(3);
  }
  if (command == 3) {
    lcd.print("Z IT!");
    myDFPlayer.play(4);
  }
  if (command == 4) {
    lcd.print("MEASURE IT!");
    myDFPlayer.play(5);
  }

  roundStartTime = millis();
}

bool isTimeUp() {
  return (millis() - roundStartTime > timeLimit);
}

void recordSuccess() {
  score++;

  lcd.clear();
  lcd.print("GOOD!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);

  myDFPlayer.play(6);
  delay(500);
}

void recordFailure(String reason) {
  lcd.clear();
  lcd.print("GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print(reason);
  lcd.setCursor(0, 2);
  lcd.print("Score: ");
  lcd.print(score);

  myDFPlayer.play(7);
  delay(1500);
}