//pin definitions
int xStep = 5;
int xDir = 4;
int yStep = 7;
int yDir = 6;
int xLimSwitch = 2;
int yLimSwitch = 3;
int magnet = 8;

boolean xForward = LOW;
boolean yForward = LOW;

//delay between steps in microseconds (sets max speed)
int stepDelay = 2000;

//position information
int x;
int y;
int xTarget;
int yTarget;
//position of limit switches (steps, 0 = minimum)
int xHome = 2100;
int yHome = 1290;
//error if position values exceed these biouds during homing
int xSafetyLimit = 5000;
int ySafetyLimit = 4000;
//(1 step = 0.2mm)
int squareSize = 140;
//position of the center of the bottom-right square
int a1x = 100;
int a1y = 15;

//indicated wheter the machine knows where it is
boolean isHomed;


unsigned long t;
unsigned long lastTime;

//for serial communication
char endMarker = ';';
const byte numChars = 32;
char receivedChars[32];
boolean newCommand;


class Stepper
{
  static const int pulseWidthMicroseconds = 5;
  
  int stepPin;
  int dirPin;
  boolean forward;
//  boolean reverse;
  
//  void pulse()
//  {
//    digitalWrite(stepPin, HIGH);
//    delayMicroseconds(pulseWidthMicroseconds);
//    digitalWrite(stepPin, LOW);
//    delayMicroseconds(pulseWidthMicroseconds);
//  }
  
  public:
  Stepper(int s, int d, boolean f)
  {
    stepPin = s;
    dirPin = d;
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    
    forward = f;
//    reverse = !forward;
    digitalWrite(stepPin, LOW);
    digitalWrite(dirPin, forward);
  }
  
  
  void step(boolean d){
    digitalWrite(dirPin, d == forward);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(pulseWidthMicroseconds);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(pulseWidthMicroseconds);
  }
  
//  void turn(int steps)
//  {
//    digitalWrite(dirPin, (steps > 0) == forward);
//
//    steps = abs(steps);
//    for(int i = 0; i < steps; i++)
//    {
//      pulse();
//    }
//  }
};





Stepper xMotor = Stepper(xStep, xDir, xForward);
Stepper yMotor = Stepper(yStep, yDir, yForward);
//Stepper xMotor;
//Stepper yMotor;
void setup() {
  Serial.begin(9600);
  newCommand = false;

//  xMotor = Stepper(xStep, xDir, HIGH);
//  yMotor = Stepper(yStep, yDir, HIGH);

  pinMode(xLimSwitch, INPUT_PULLUP);
  pinMode(yLimSwitch, INPUT_PULLUP);
  pinMode(magnet, OUTPUT);
  digitalWrite(magnet, LOW);


  x = 0;
  y = 0;
  xTarget = x;
  yTarget = y;

  isHomed = false;

  lastTime = micros();



//  delay(3000);
//  homeXY();
  Serial.println("ready");
}

void loop() {
  // put your main code here, to run repeatedly:
//  Serial.println("x forward");
//  xMotor.turn(600);
//  delay(1000);
//  
//  Serial.println("x reverse");
//  xMotor.turn(-600);
//  delay(1000);
//
//  Serial.println("y forward");
//  yMotor.turn(600);
//  delay(1000);
//  
//  Serial.println("y reverse");
//  yMotor.turn(-600);
//  delay(5000);


  //check if a command has been recieved over serial and act on it
  receiveCommand();
  parseCommand();


  //periodically move the motors
//  t = micros();
//  if(isHomed && t - lastTime >= stepDelay){
//    //update the time, but avoid sending many steps in quick succession if there is a delay
//    while(t - lastTime >= stepDelay) {lastTime += stepDelay;};
//
//
//    if(x < xTarget){
//      xMotor.step(true);
//      x++;
//    }else if (x > xTarget){
//      xMotor.step(false);
//      x--;
//    }
//    
//    if(y < yTarget){
//      yMotor.step(true);
//      y++;
//    }else if (y > yTarget){
//      yMotor.step(false);
//      y--;
//    }
//
//
//    if(digitalRead(xLimSwitch)){
//      x = xHome;
//    }
//    if(digitalRead(yLimSwitch)){
//      y = yHome;
//    }
//
//    
//  }
  
}




void homeXY(){
  Serial.println("homing");

  //x
  lastTime = micros();
  while(!digitalRead(xLimSwitch)){
    t = micros();
    if(t - lastTime > stepDelay){
      while(t - lastTime >= stepDelay) {lastTime += stepDelay;};
      xMotor.step(true);
      x++;

      //stop the program if the limit switch is not detected
      if(x >= xSafetyLimit){
        Serial.println("Error homing X axis");
        while(true);
      }
    }
    
  }
  x = xHome;
  xTarget = x;
  Serial.println("X axis homed");

  delay(500);
  
  //y
  while(!digitalRead(yLimSwitch)){
    t = micros();
    if(t - lastTime > stepDelay){
      while(t - lastTime >= stepDelay) {lastTime += stepDelay;};
      yMotor.step(true);
      y++;

      //stop the program if the limit switch is not detected
      if(y >= ySafetyLimit){
        Serial.println("Error homing Y axis");
        while(true);
      }
    }
    
  }
  y = yHome;
  yTarget = y;
  Serial.println("y axis homed");

  isHomed = true;

  //
  delay(500);
  xTarget = xHome - 20;
  yTarget = yHome - 20;
  moveToTarget();
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

    switch(receivedChars[0]){
      case 'H': //home axes
        homeXY();
        break;
      case 'X': //go to x position
        n = getNum();
        if(n >= 0 and n <= xHome){
          xTarget = n;
          moveToTarget();
        }else{
          Serial.println("X position out of bounds");
        }
        break;
      case 'Y': //go to y position
        n = getNum();
        if(n >= 0 and n <= yHome){
          yTarget = n;
          moveToTarget();
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
  Serial.println(numStr);
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
  xTarget = xSquare;
  yTarget = ySquare;
  moveToTarget();
  
}

//
void moveToTarget(){
  //don't move without a valid home position
  if(!isHomed){
    Serial.println("no valid home position");
    return;
  }

  
  while(!(x == xTarget && y == yTarget)){
    //TODO: overflow protection
    t = micros();
    if(t - lastTime >= stepDelay){
      //update the time, but avoid sending many steps in quick succession if there is a delay
      while(t - lastTime >= stepDelay) {lastTime += stepDelay;};
      if(x < xTarget){
        xMotor.step(true);
        x++;
      }else if (x > xTarget){
        xMotor.step(false);
        x--;
      }
      
      if(y < yTarget){
        yMotor.step(true);
        y++;
      }else if (y > yTarget){
        yMotor.step(false);
        y--;
      }
  
  
      if(digitalRead(xLimSwitch)){
        x = xHome;
      }
      if(digitalRead(yLimSwitch)){
        y = yHome;
      }
    }

    
  }
}
