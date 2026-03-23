void setup() {
  int n = 0; //number of rounds successfully passed
  const int DIFFICULTY = 0; //difficulty integer which, when modified, changes the rate at which the game gets faster (when it is zero, every round is the same length)
  const int NUM_STATES = 6;
  const char* states[NUM_STATES] = {"|0>", "|1>", "|+>", "|->", "|i>", "|-i>"};
  const int NUM_COMMANDS = 6;
  const char* commands[NUM_COMMANDS] = {"INIT_IT", "X_IT", "Y_IT", "Z_IT", "H_IT", "MEAS_IT"};
  //state and command arrays will be useful because states[0] or iterating states[i] 
  //is easier to keep track of and more memory efficient than using strings repeatedly
  bool LEDS[NUM_STATES] = {false, false, false, false, false, false};
  bool INIT_LED = false; //LED on the controller that flashes whn turned on and held on when qubit is initialized
}

void loop() {
  // put your main code here, to run repeatedly:

}
