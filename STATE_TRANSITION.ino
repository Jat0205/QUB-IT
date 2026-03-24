// global variable definition
int r = 0; //number of rounds successfully passed
const int DIFFICULTY = 0; //difficulty integer which, when modified, changes the rate at which the game gets faster (when it is zero, every round is the same length)
int qubitState = 0; //current state of the qubit

//STATE TRANSITION LOOKUP TABLE
// transition[operation][current_state] = new_state
// operations:   0=intialize, 1=apply_x_gate, 2=apply_y_gate, 3=apply_z_gate 4=apply_h_gate, 5=perform_x_measurement, 6=perform_y_measurement, 7=perform_z_measurement
// States:  0=|0>, 1=|1>, 2=|+>, 3=|->, 4=|i>, 5=|-i>
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
  {1, 0, -3, -3, -3, -3},  // ZMEAS: every -3 needs sep. function to collapse to 1 or 0
};


bool LEDS[NUM_STATES] = {false, false, false, false, false, false};
bool INIT_LED = false; //LED on the controller that flashes whn turned on and held on when qubit is initialized

//rot_enc globals
int x = 0;
int y = 0;
int z = 0;
int lastStateX = LOW;
int lastStateY = LOW;
int lastStateZ = LOW;

int coin = 0;


void setup() {
  pinMode(2, INPUT); //rot_encX
  pinMode(4, INPUT); //rot_encY
  pinMode(7, INPUT); //rot_encZ
  pinMode(8, OUTPUT);//output LED to verify rot_enc pulsing
  pinMode(A2, INPUT); //measure x button
  pinMode(A1, INPUT); //measure y button
  pinMode(A0, INPUT); //measure z button
  pinMode(3, OUTPUT); //|0> output
  pinMode(9, OUTPUT); //|1> output
  pinMode(5, OUTPUT); //|+> output
  pinMode(6, OUTPUT); //|-> output
  pinMode(10, OUTPUT); //|i> output
  pinMode(11, OUTPUT); //|-i> output
  lastStateX = digitalRead(2);
  lastStateY = digitalRead(4);
  lastStateZ = digitalRead(7);
  qubitState = 0;
}

void loop() {
  //rot_encX, Y, and Z, input detection
  detect_rotation();

  //state transitions for x, y, and z inputs
  detect_Xgate();
  detect_Ygate();
  detect_Zgate();

  diode_output(qubitState);
  
  //measure it input register
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

void detect_rotation(){
  int currentStateX = digitalRead(2);
  int currentStateY = digitalRead(4);
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

//function detects x gate and adjusts state accordingly
bool detect_Xgate(){
 if(x == 5){
    qubitState = state_transition[1][qubitState];
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
    x = 0;
  } 
}

//function detects y gate and adjusts state accordingly
void detect_Ygate(){
  if(y == 5){
    qubitState = state_transition[2][qubitState];
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
    y= 0;
  }
}

//function detects z gate and adjusts state accordingly
void detect_Zgate(){
 if(z == 5){
    qubitState = state_transition[3][qubitState];
    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
    z = 0;
  }
}

void diode_output(int qubitState){
  digitalWrite(3, LOW);
  digitalWrite(9, LOW);
  digitalWrite(5, LOW);
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
      digitalWrite(5, HIGH);
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
    case -1:

      break;
    case -2:

      break;
    case -3:

      break;
  }
}

//function that takes the current state and returns an int corresponding to new state.
//should be called in main qubitState = collapse_it(qubitState)
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
}

