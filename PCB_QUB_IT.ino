// ================================================================
//  QUB-IT  —  A Quantum Bop-It Game
// ================================================================

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include "DFRobotDFPlayerMini.h"

// ================= SOFT SERIAL FOR DFPLAYER =================
#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/*rx =*/4, /*tx =*/5);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

DFRobotDFPlayerMini myDFPlayer;

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ================= PIN DEFINITIONS =================
const int ROTENCX    = 2;
const int ROTENCY    = 0;
const int ROTENCZ    = 7;
const int DEBUGLED   = 8;
const int PHOTODIODE = A3;
const int MEASUREX   = A2;
const int MEASUREY   = A1;
const int MEASUREZ   = A0;

const int LED_PINS[6] = {
  3,   // index 0 → |0>   (PWM)
  9,   // index 1 → |1>   (PWM)
  1,   // index 2 → |+>   (digital only)
  6,   // index 3 → |->   (PWM)
  10,  // index 4 → |i>   (PWM)
  11   // index 5 → |-i>  (PWM)
};

// ================= QUBIT PHYSICS ENGINE =================
int qubitState = 0;

const int NUM_OPS    = 8;
const int NUM_STATES = 6;
const int state_transition[NUM_OPS][NUM_STATES] = {
  {0, 0, 0, 0, 0, 0},
  {1, 0, 2, 3, 5, 4},
  {1, 0, 3, 2, 4, 5},
  {0, 1, 3, 2, 5, 4},
  {2, 3, 0, 1, 5, 4},        // H (unused)
  {-1, -1, 2, 3, -1, -1},
  {-2, -2, -2, -2, 4, 5},
  {0, 1, -3, -3, -3, -3},
};

int collapse_it(int state) {
  int coin = TCNT0 & 1;
  if (state == -1) return (coin == 0) ? 2 : 3;
  if (state == -2) return (coin == 0) ? 4 : 5;
  if (state == -3) return (coin == 0) ? 0 : 1;
  return state;
}

// ================= ENCODER GLOBALS =================
volatile int encX = 0, encY = 0, encZ = 0;
int lastStateX = LOW, lastStateY = LOW, lastStateZ = LOW;

// ================= GAME STATE MACHINE =================
enum GameState {
  GS_IDLE,
  GS_COMMAND,
  GS_WAIT_INPUT,
  GS_SUCCESS,
  GS_GAME_OVER
};
GameState gameState = GS_IDLE;

enum Command {
  CMD_INIT,
  CMD_X,
  CMD_Y,
  CMD_Z,
  CMD_MEASURE
};
Command currentCmd = CMD_INIT;

// Forward declarations
//void playCommand(Command cmd);
bool checkForWrongInput(Command cmd);
bool checkCorrectInput(Command cmd);
Command pickRandomCommand();
const char* commandText(Command cmd);

int  score           = 0;
int  dim_value       = 255;
bool ledOn           = false;
int  currentLedPin   = -1;

unsigned long roundStart    = 0;
unsigned long roundDuration = 10000;
const unsigned long MIN_ROUND_TIME = 2500;

const int ENCODER_THRESHOLD = 5;

// ================= DFPlayer TRACK MAP =================
const int TRACK_INIT    = 7;
const int TRACK_X       = 10;
const int TRACK_Y       = 8;
const int TRACK_Z       = 4;
const int TRACK_MEASURE = 2;
const int TRACK_STARTUP = 1;

// ================= DEBUG LED =================
void flashDebugLed() {
  digitalWrite(DEBUGLED, HIGH);
  delay(80);
  digitalWrite(DEBUGLED, LOW);
}

// ================= LED HELPERS =================
void ledWrite(int pin, int value) {
  if (pin == 1) {
    digitalWrite(pin, value > 127 ? HIGH : LOW);
  } else {
    analogWrite(pin, value);
  }
}

void allLedsOff() {
  for (int i = 0; i < 6; i++) {
    ledWrite(LED_PINS[i], 0);
  }
  ledOn = false;
  currentLedPin = -1;
}

void showQubitLed() {
  allLedsOff();
  currentLedPin = LED_PINS[qubitState];
  dim_value = 255;
  ledWrite(currentLedPin, 255);
  ledOn = true;
}

void dimLed() {
  if (!ledOn || currentLedPin < 0) return;
  unsigned long elapsed = millis() - roundStart;
  if (elapsed >= roundDuration) {
    dim_value = 0;
  } else {
    dim_value = map(elapsed, 0, roundDuration, 255, 0);
  }
  ledWrite(currentLedPin, dim_value);
  if (dim_value == 0) ledOn = false;
}

// ================= ENCODER POLLING =================
void pollEncoders() {
  int cx = digitalRead(ROTENCX);
  int cy = digitalRead(ROTENCY);
  int cz = digitalRead(ROTENCZ);

  if (cx == HIGH && lastStateX == LOW) encX++;
  lastStateX = cx;

  if (cy == HIGH && lastStateY == LOW) encY++;
  lastStateY = cy;

  if (cz == HIGH && lastStateZ == LOW) encZ++;
  lastStateZ = cz;
}

void resetEncoders() {
  encX = 0; encY = 0; encZ = 0;
}

// ================= SPEAKER =================
/*void playCommand(Command cmd) {
  switch (cmd) {
    case CMD_INIT:    myDFPlayer.play(TRACK_INIT);    break;
    case CMD_X:       myDFPlayer.play(TRACK_X);       break;
    case CMD_Y:       myDFPlayer.play(TRACK_Y);       break;
    case CMD_Z:       myDFPlayer.play(TRACK_Z);       break;
    case CMD_MEASURE: myDFPlayer.play(TRACK_MEASURE); break;
  }
}*/

// ================= LCD HELPERS =================
void lcdShowCommand(const char* text) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(text);
}

// ================= INPUT DETECTION =================
bool checkForWrongInput(Command cmd) {
  int mx = digitalRead(MEASUREX);
  int my = digitalRead(MEASUREY);
  int mz = digitalRead(MEASUREZ);
  int photo = analogRead(PHOTODIODE);
  bool laserHit = (photo >= 512);

  switch (cmd) {
    case CMD_INIT:
      if (mx == HIGH || my == HIGH || mz == HIGH) return true;
      if (encX > 2 || encY > 2 || encZ > 2) return true;
      break;
    case CMD_X:
      if (mx == HIGH || my == HIGH || mz == HIGH) return true;
      if (laserHit) return true;
      if (encY > ENCODER_THRESHOLD || encZ > ENCODER_THRESHOLD) return true;
      break;
    case CMD_Y:
      if (mx == HIGH || my == HIGH || mz == HIGH) return true;
      if (laserHit) return true;
      if (encX > ENCODER_THRESHOLD || encZ > ENCODER_THRESHOLD) return true;
      break;
    case CMD_Z:
      if (mx == HIGH || my == HIGH || mz == HIGH) return true;
      if (laserHit) return true;
      if (encX > ENCODER_THRESHOLD || encY > ENCODER_THRESHOLD) return true;
      break;
    case CMD_MEASURE:
      if (laserHit) return true;
      if (encX > 2 || encY > 2 || encZ > 2) return true;
      break;
  }
  return false;
}

bool checkCorrectInput(Command cmd) {
  switch (cmd) {
    case CMD_INIT: {
      int photo = analogRead(PHOTODIODE);
      if (photo >= 512) {
        qubitState = state_transition[0][qubitState];
        return true;
      }
      return false;
    }
    case CMD_X:
      if (encX >= ENCODER_THRESHOLD) {
        qubitState = state_transition[1][qubitState];
        return true;
      }
      return false;
    case CMD_Y:
      if (encY >= ENCODER_THRESHOLD) {
        qubitState = state_transition[2][qubitState];
        return true;
      }
      return false;
    case CMD_Z:
      if (encZ >= ENCODER_THRESHOLD) {
        qubitState = state_transition[3][qubitState];
        return true;
      }
      return false;
    case CMD_MEASURE: {
      int mx = digitalRead(MEASUREX);
      int my = digitalRead(MEASUREY);
      int mz = digitalRead(MEASUREZ);
      if (mx == HIGH) {
        qubitState = state_transition[5][qubitState];
        qubitState = collapse_it(qubitState);
        return true;
      }
      if (my == HIGH) {
        qubitState = state_transition[6][qubitState];
        qubitState = collapse_it(qubitState);
        return true;
      }
      if (mz == HIGH) {
        qubitState = state_transition[7][qubitState];
        qubitState = collapse_it(qubitState);
        return true;
      }
      return false;
    }
  }
  return false;
}

// ================= COMMAND GENERATION =================
Command pickRandomCommand() {
  int r = random(0, 5);
  switch (r) {
    case 0:  return CMD_INIT;
    case 1:  return CMD_X;
    case 2:  return CMD_Y;
    case 3:  return CMD_Z;
    default: return CMD_MEASURE;
  }
}

const char* commandText(Command cmd) {
  switch (cmd) {
    case CMD_INIT:    return "INITIALIZE IT!";
    case CMD_X:       return "X IT!";
    case CMD_Y:       return "Y IT!";
    case CMD_Z:       return "Z IT!";
    case CMD_MEASURE: return "MEASURE IT!";
  }
  return "";
}

// ================================================================
//  SETUP
// ================================================================
void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("QUB-IT");
  lcd.setCursor(0, 1);
  lcd.print("Point laser to start");

  FPSerial.begin(9600);
  /*if (!myDFPlayer.begin(FPSerial, true, true)) {
    lcd.clear();
    lcd.print("DFPlayer FAIL");
    while (true);
  }
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(30);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.play(TRACK_STARTUP);*/

  pinMode(ROTENCX, INPUT);
  pinMode(ROTENCY, INPUT);
  pinMode(ROTENCZ, INPUT);
  pinMode(DEBUGLED, OUTPUT);
  pinMode(MEASUREX, INPUT);
  pinMode(MEASUREY, INPUT);
  pinMode(MEASUREZ, INPUT);
  for (int i = 0; i < 6; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    ledWrite(LED_PINS[i], 0);
  }

  lastStateX = digitalRead(ROTENCX);
  lastStateY = digitalRead(ROTENCY);
  lastStateZ = digitalRead(ROTENCZ);
  randomSeed(analogRead(A5));

  gameState = GS_IDLE;
  qubitState = 0;
  score = 0;
}

// ================================================================
//  MAIN LOOP
// ================================================================
void loop() {
  pollEncoders();

  switch (gameState) {

    case GS_IDLE: {
      int photo = analogRead(PHOTODIODE);
      if (photo >= 512) {
        flashDebugLed();
        qubitState = 0;
        score = 0;
        roundDuration = 10000;
        allLedsOff();
        showQubitLed();
        score++;
        gameState = GS_SUCCESS;
      }
      break;
    }

    case GS_COMMAND: {
      currentCmd = pickRandomCommand();
      lcdShowCommand(commandText(currentCmd));
      //playCommand(currentCmd);
      resetEncoders();
      roundStart = millis();
      showQubitLed();
      gameState = GS_WAIT_INPUT;
      break;
    }

    case GS_WAIT_INPUT: {
      if (millis() - roundStart >= roundDuration) {
        gameState = GS_GAME_OVER;
        break;
      }
      dimLed();
      if (checkForWrongInput(currentCmd)) {
        flashDebugLed();
        gameState = GS_GAME_OVER;
        break;
      }
      if (checkCorrectInput(currentCmd)) {
        flashDebugLed();
        gameState = GS_SUCCESS;
      }
      break;
    }

    case GS_SUCCESS: {
      showQubitLed();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("GOOD!");
      lcd.setCursor(0, 1);
      lcd.print("Score: ");
      lcd.print(score);

      if (score >= 99) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("VICTORY!");
        lcd.setCursor(0, 1);
        lcd.print("Final Score: 99");
        lcd.setCursor(0, 2);
        lcd.print("You are a quantum");
        lcd.setCursor(0, 3);
        lcd.print("master!");
        gameState = GS_GAME_OVER;
        break;
      }

      if (roundDuration > MIN_ROUND_TIME) {
        roundDuration -= 500;
        if (roundDuration < MIN_ROUND_TIME) {
          roundDuration = MIN_ROUND_TIME;
        }
      }

      delay(400);
      score++;
      gameState = GS_COMMAND;
      break;
    }

    case GS_GAME_OVER: {
      allLedsOff();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("GAME OVER");
      lcd.setCursor(0, 1);
      lcd.print("Score: ");
      lcd.print(score);
      lcd.setCursor(0, 2);
      lcd.print("Laser to restart");

      while (true) {
        int photo = analogRead(PHOTODIODE);
        if (photo >= 512) {
          flashDebugLed();
          delay(500);
          break;
        }
      }

      qubitState = 0;
      score = 0;
      roundDuration = 10000;
      resetEncoders();
      allLedsOff();
      showQubitLed();
      score++;
      gameState = GS_SUCCESS;
      break;
    }
  }
}