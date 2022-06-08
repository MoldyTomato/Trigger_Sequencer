#define TRIGGER 3 //Jack for input

#define OUTPUT_A 18 //STEP 1
#define OUTPUT_B 19 //STEP 2
#define OUTPUT_C 20 //STEP 3
#define OUTPUT_D 21 //STEP 4
#define OUTPUT_E 22 //STEP 5
#define OUTPUT_F 23 //STEP 6
#define OUTPUT_G 14 //STEP 7
#define OUTPUT_H 15 //STEP 8
int OUTPUTS[8] = {OUTPUT_A, OUTPUT_B, OUTPUT_C, OUTPUT_D, OUTPUT_E, OUTPUT_F, OUTPUT_G, OUTPUT_H};

#define SWITCH_A 12 //STEP 1
#define SWITCH_B 11 //STEP 2
#define SWITCH_C 10 //STEP 3
#define SWITCH_D  9 //STEP 4
#define SWITCH_E  8 //STEP 5
#define SWITCH_F  7 //STEP 6
#define SWITCH_G  6 //STEP 7
#define SWITCH_H  5 //STEP 8
int SWITCHES[8] = {SWITCH_A,SWITCH_B,SWITCH_C,SWITCH_D,SWITCH_E,SWITCH_F,SWITCH_G,SWITCH_H};
int _currentSwitchStates[8]={0,0,0,0,0,0,0,0}; //switches are high when step off
int _sumSwitches;

#define RESET 2
#define HOLD 4

#define MODE_A 1//MODE 1
#define MODE_B 16//MODE 2

 

#define MODE_0 0 //NORMAL MODE: No skips
#define MODE_SKIP 1 //MODE A: Steps that are off will be skipped
#define MODE_CHANGE 2 //MODE B: Step is held until the next trigger arrives

int _currentMode;
int _direction;
volatile int _currentStep;
int hold_step;

#define STEPS 8 
void setup() {
  //set input
  pinMode(TRIGGER, INPUT);
  //set trigger outputs
  for(int i=0;i<8;i++){
    pinMode(OUTPUTS[i], OUTPUT);  
  }
  //turn all on to signify boot sequence
  for(int i=0;i<8;i++){
    digitalWrite(OUTPUTS[i],HIGH);
    delay(50);
  }
  //set step switches as inputs
  for(int i=0;i<8;i++){
    pinMode(SWITCHES[i], INPUT_PULLUP);
  }
  //set reset switch as input
  pinMode(RESET, INPUT_PULLUP);
  //set hold switch as input
  pinMode(HOLD, INPUT_PULLUP);
  //set mode switch as input
  pinMode(MODE_A, INPUT_PULLUP);
  pinMode(MODE_B, INPUT_PULLUP);

  

  //attach interrupts
  attachInterrupt(digitalPinToInterrupt(TRIGGER), trigger, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RESET), reset, FALLING); //reset is pullup, so switch closed is low

  //set currentStep to 0
  _currentStep = 0;
  //read the hold switch
  hold_step = !digitalRead(HOLD);

  //read all switches
  _sumSwitches = 0;
  for (int i=0;i<8;i++){
    _currentSwitchStates[i] = digitalRead(SWITCHES[i]); //switches are low when closed, high means active
    _sumSwitches +=_currentSwitchStates[i];
  }

  //set default direction
  _direction = 1;
  //turn all off to signify end of boot sequence
  for(int i=0;i<8;i++){
    digitalWrite(OUTPUTS[i],LOW);
    delay(50);
  }

  //Serial.begin(9600);
}

void loop() {
  
  int pin_A = digitalRead(MODE_A);
  int pin_B = digitalRead(MODE_B);
  ////Serial.print(pin_A); //Serial.println(pin_B);
  if(pin_A == HIGH && pin_B == HIGH && _currentMode != MODE_0){
    //Serial.println("MODE 0");
    _currentMode = MODE_0;
  }else if(pin_A == LOW && pin_B == HIGH && _currentMode != MODE_SKIP ){
    //Serial.println("MODE SKIP");
    _currentMode = MODE_SKIP;
  }else if(pin_B == LOW && pin_A == HIGH && _currentMode != MODE_CHANGE){
    //Serial.println("MODE CHANGE");
    _currentMode = MODE_CHANGE;
  }else{
    ////Serial.println("");
  }
  int h = !digitalRead(HOLD);
  if(h != hold_step){
    if(h==LOW){
      for(int i=0;i<8;i++){
        digitalWrite(OUTPUTS[i],LOW);
      }
    }
    hold_step=h;
  }
  //read switches
  _sumSwitches = 0;
  for(int i=0; i<8;i++){
    _currentSwitchStates[i] = digitalRead(SWITCHES[i]);
    _sumSwitches += _currentSwitchStates[i];
  }
}
void reset(){
  _currentStep = 0;
  _direction = 1;
  //Serial.println("RESET");
}
void trigger(){
  if(digitalRead(TRIGGER)){
    //Serial.println("Rising");
    TriggerHigh();
  }else{
    //Serial.println("Falling");
    TriggerLow();
  }
}
void TriggerHigh(){
  //turn off previous Steps
  for(int i=0;i<8;i++){
    digitalWrite(OUTPUTS[i], LOW);
  }
  if(_currentMode == MODE_0){
    digitalWrite(OUTPUTS[_currentStep], _currentSwitchStates[_currentStep]);
  }
  else if(_currentMode == MODE_SKIP && _sumSwitches != 0){
    digitalWrite(OUTPUTS[_currentStep],HIGH);
  }
  else if(_currentMode == MODE_CHANGE){
    digitalWrite(OUTPUTS[_currentStep],HIGH);
  }
  //digitalWrite(OUTPUTS[_currentStep], HIGH);
}
void TriggerLow(){
  //turn of the step if not holding
  if(hold_step!=HIGH){
    digitalWrite(OUTPUTS[_currentStep], LOW);
  }
  //advance step
  if(_currentMode == MODE_0){
    _currentStep = (_currentStep+1)%STEPS;
  }
  else if(_currentMode == MODE_SKIP){
    for(int i=1;i<=8;i++){
      if(_currentSwitchStates[(_currentStep+i)%STEPS]==1){ //get the next possible step
        _currentStep = (_currentStep+i)%STEPS; //if this is the currentstep itself it is either the only switch closed or all steps are off, this is checked in triggerRising
        break;
      }
    }
  }else if(_currentMode == MODE_CHANGE){
    //if the switch on currentStep is closed, change direction
    if(_currentSwitchStates[_currentStep] == 0){
      _direction*=-1;
    }
    _currentStep = (_currentStep + _direction + STEPS)%STEPS;
  }
}
