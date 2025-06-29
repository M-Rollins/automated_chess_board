//pin definitions
int xStep = 5;
int xDir = 4;
int yStep = 7;
int yDir = 6;
int xms1 = 10;
int xms2 = 9;
int yms1 = 12;
int yms2 = 11;
int xLimSwitch = 2;
int yLimSwitch = 3;
int magnet = 8;

boolean xForward = LOW;
boolean yForward = LOW;

//delay between steps in microseconds (sets max speed)
//int stepDelay = 2000;
unsigned long xStepDelay, yStepDelay;
//int MAX_STEP_DELAY = 10000;
//int MIN_STEP_DELAY = 2000;
//int DELAY_INCREMENT = 10;

//// steps per second
//float vx, vy;
//float V_MIN = 200;
//float V_MAX = 500;
////steps/s^2
//float ACCEL = 500;

////position information
//int x;
//int y;
//int xTarget;
//int yTarget;
//position of limit switches (steps, 0 = minimum)
int xHome = 2100;
int yHome = 1290;
////error if position values exceed these biouds during homing
//int xSafetyLimit = 5000;
//int ySafetyLimit = 4000;
//(1 step = 0.2mm)
int squareSize = 140;
//position of the center of the bottom-right square
int a1x = 100;
int a1y = 15;

//indicated wheter the machine knows where it is
//boolean isHomed;


unsigned long t;
unsigned long xLastTime, yLastTime;

//for serial communication
char endMarker = ';';
const byte numChars = 32;
char receivedChars[32];
boolean newCommand;


#include "MotorControl.h"

//StepperMotor xMotor = StepperMotor(xStep, xDir, xForward);
//StepperMotor yMotor = StepperMotor(yStep, yDir, yForward);
StepperMotor xMotor;
StepperMotor yMotor;
Axis x, y;
void setup() {
  Serial.begin(9600);
  newCommand = false;

  xMotor = StepperMotor(xStep, xDir, xms1, xms2, xForward);
  yMotor = StepperMotor(yStep, yDir, yms1, yms2, yForward);
  xMotor.setMicrostepping(2);
  yMotor.setMicrostepping(2);
  x = Axis(xMotor, xHome, xLimSwitch);
  y = Axis(yMotor, yHome, yLimSwitch);

//  pinMode(xLimSwitch, INPUT_PULLUP);
//  pinMode(yLimSwitch, INPUT_PULLUP);
  pinMode(magnet, OUTPUT);
  digitalWrite(magnet, LOW);


  x.setTarget(0);
  y.setTarget(0);

//  isHomed = false;

//  delay(3000);
//  homeXY();
  Serial.println("ready");
}

void loop() {
  //check if a command has been recieved over serial and act on it
  receiveCommand();
  parseCommand();
  delay(100);

  //move
  if(x.isHomed && y.isHomed){
    while(!(x.atTarget() && y.atTarget())){
      x.updateAxis();
      y.updateAxis();
    }
  }else if(!(x.atTarget() && y.atTarget())){
    Serial.println("no valid home position");
  }
  
  
  
}




void homeXY(){
  Serial.println("homing");

  //x
  if(x.homeAxis()){
    Serial.println("X axis homed");
  }else{
    Serial.println("Error homing X axis");
    while(true){}
  }

  delay(500);
  
  //y
  if(y.homeAxis()){
    Serial.println("Y axis homed");
  }else{
    Serial.println("Error homing Y axis");
    while(true){}
  }
  
  //
  delay(500);
  x.setTarget(xHome - 20);
  y.setTarget(yHome - 20);

//  moveToTarget();
}



void receiveCommand(){
  static byte ndx = 0;
  char rc;
  
  while (Serial.available() > 0 && newCommand == false) {
      rc = Serial.read();

      if (rc != endMarker) {
          receivedChars[ndx] = rc;
          ndx++;
          if (ndx >= numChars) {
              ndx = numChars - 1;
          }
      }
      else {
          receivedChars[ndx] = '\0'; // terminate the string
          ndx = 0;
          newCommand = true;
      }
  }
}


void parseCommand(){
  if(newCommand){
    Serial.print("received command: ");
    Serial.println(receivedChars);
    int n;

    //first character determines the type of command
    switch(receivedChars[0]){
      case 'H': //home axes
        homeXY();
        break;
      case 'X': //go to x position
        n = getNum();
        if(n >= 0 and n <= xHome){
          x.setTarget(n);
        }else{
          Serial.println("X position out of bounds");
        }
        break;
      case 'Y': //go to y position
        n = getNum();
        if(n >= 0 and n <= yHome){
          y.setTarget(n);
        }else{
          Serial.println("Y position out of bounds");
        }
        break;

      case 'M': //go to square
        parseMove();
        break;

      default:
        Serial.println("invalid command");
      break;
    }
    
    newCommand = false;
  }
}


//search the received line for an integer (starting at the second character)
int getNum(){
  String numStr = "";
  for(int i = 1; i < numChars; i ++){
    if(isDigit(receivedChars[i])){
      numStr += receivedChars[i];
    }else{
      break;
    }
  }
//  Serial.println(numStr);
  return(numStr.toInt());
}


void parseMove(){
  int coords[4];
  int cIdx = 0;
  int n = 0;
  
  String numStr = "";
  for(int i = 1; i < numChars && cIdx < 4; i ++){
    if(isDigit(receivedChars[i])){
      numStr += receivedChars[i];
      if(++n == 2){
        coords[cIdx++] =  numStr.toInt();
        n = 0;
        numStr = "";
      }
    }else{
      Serial.println("Invalid move command");
      return;
    }
  }
  if(cIdx < 4){
    Serial.println("Invalid move command");
      return;
  }

  makeMove(coords[0], coords[1], coords[2], coords[3]);
}



//move a piece form one square to another
void makeMove(int f0, int r0, int f1, int r1){
  goToSquare(f0, r0);
  delay(500);
  digitalWrite(magnet, HIGH);
  delay(500);
  goToSquare(f1, r1);
  delay(500);
  digitalWrite(magnet, LOW);
  
}

//go to file f, square r (a1 = (0, 0))
void goToSquare(int f, int r){
  int xSquare = f * squareSize + a1x;
  int ySquare = r * squareSize + a1y;
  if(xSquare < 0 || xSquare > xHome || ySquare < 0 || ySquare > yHome){
    Serial.println("Square out of bounds");
    return;
  }

  x.setTarget(xSquare);
  y.setTarget(ySquare);
  
}
