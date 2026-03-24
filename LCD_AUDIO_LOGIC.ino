#include <LiquidCrystal_I2C.h>

//LCD setup
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20 column and 4 rows

//Pin setup
const int AUDIO_PIN = 0; //temporary pin #, can change in the future; replace with DFPlayer audio later?

//Game variables
int score = 0; //keeps track of player score
unsigned long roundStartTime = 0; //stores when a round starts
unsigned long timeLimit = 3000; //time allowed per round (ms); can be adjusted based on difficulty or score

//Setup
void setup() {
  lcd.init(); //initialize LCD
  lcd.backlight(); //turn on backlight

  pinMode(AUDIO_PIN, OUTPUT);

  lcd.setCursor(0,0);
  lcd.print("READY TO PLAY"); 

  //later initialize DFPlayer here
}

//loop(), mainly unused because other files will call these functions
void loop() {
  updateTimerDisplay(); //update timer display while a round is active
}

//starts a new round, input: command (0=INIT, 1=X, 2=Y, 3=Z, 4=MEASURE)
void startRound (int command) {
  lcd.clear();
  lcd.setCursor(0,0);

  //display command on LCD
  if (command == 0) {
    lcd.print("INITIALIZE IT!");
  }
  else if (command == 1){
    lcd.print ("X IT!");
  }
  else if (command == 2){
    lcd.print ("Y IT!");
  }
  else if (command == 3){
    lcd.print ("Z IT!");
  }
  else if (command == 4){
    lcd.print ("MEASURE IT!");
  }

  //Audio (temporary), HIGH/LOW signal is justa placeholder to indicate when audio should play
  digitalWrite(AUDIO_PIN, HIGH);
  delay(200);
  digitalWrite(AUDIO_PIN, LOW);

  //later replace the above with DFPlayer specific code

  roundStartTime = millis(); //start timer for this round
}

//Called when player performs correct action
void recordSuccess() {
  score++; //increase score

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GOOD!");

  lcd.setCursor(0,1);
  lcd.print("Score: ");
  lcd.print(score);

  delay(500);

  //later add success sound here
}

//called when player fails (wrong input/timeout)
void recordFailure(String reason) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("GAME OVER");

  lcd.setCursor(0,1);
  lcd.print(reason);

  lcd.setCursor(0,2);
  lcd.print("Score: ");
  lcd.print(score);

  //temporary audio section, later replace with DFPlayer code
  digitalWrite(AUDIO_PIN, HIGH);
  delay(600);
  digitalWrite(AUDIO_PIN, LOW);
}

//Return true if time limit has been exceeded
bool isTimeUp() {
  if (millis() - roundStartTime > timeLimit) {
    return true;
  }
  return false;
}

//Updates the countdown timer shown on the LCD
void updateTimerDisplay() {
  // Calculate how much time is left
  long timeLeft = (long)timeLimit - (long)(millis() - roundStartTime);

  // Prevent negative numbers from showing
  if (timeLeft < 0) {
    timeLeft = 0;
  }

  // Display remaining time on the LCD
  lcd.setCursor(0, 1);
  lcd.print("Time Left: ");
  lcd.print(timeLeft / 1000.0);
  lcd.print("s   ");
}
//resets score and prepares for a new game
void resetGame() {
  score = 0;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("NEW GAME");

  delay(500);

  //later can implement DFPlayer code
}