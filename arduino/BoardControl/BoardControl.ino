//pin definitions
byte xStep = 5;
byte xDir = 4;
byte yStep = 7;
byte yDir = 6;
byte xms1 = 10;
byte xms2 = 9;
byte yms1 = 12;
byte yms2 = 11;
byte xLimSwitch = 2;
byte yLimSwitch = 3;
byte magnet = 8;

const char PAWN = 1;
const char KNIGHT = 2;
const char BISHOP = 3;
const char ROOK = 4;
const char QUEEN = 5;
const char KING = 6;

const boolean xForward = LOW;
const boolean yForward = LOW;
const byte MICROSTEPPING = 4;

//position of limit switches (steps, 0 = minimum)
const int xHome = 2100;
const int yHome = 1290;

//(1 step = 0.2mm)
const int squareSize = 140;
//position of the center of the bottom-right square
const int xOffset = 100;
const int yOffset = 15;
//square coordinates of a1
const int a1x = 4;
const int a1y = 1;

const byte boardWidth = 15;
const byte boardHeight = 10;
char boardState[boardHeight][boardWidth];
const char STARTING_POSITION[boardHeight][boardWidth] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK, 0, 0, 0},
  {0, 0, 0, 0, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, -PAWN, 0, 0, 0},
  {0, 0, 0, 0, -ROOK, -KNIGHT, -BISHOP, -QUEEN, -KING, -BISHOP, -KNIGHT, -ROOK, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
const char EMPTY_POSITION[boardHeight][boardWidth] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {ROOK, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -ROOK},
  {KNIGHT, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KNIGHT},
  {BISHOP, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -BISHOP},
  {KING, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KING},
  {QUEEN, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -QUEEN},
  {BISHOP, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -BISHOP},
  {KNIGHT, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KNIGHT},
  {ROOK, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -ROOK},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};


unsigned long t;
unsigned long xLastTime, yLastTime;

//for serial communication
char endMarker = ';';
const byte numChars = 32;
char receivedChars[32];
byte charIdx; //indicates the next recieved character to parse
byte msgLen;
boolean newCommand;
//for move commands following a path
const byte MAX_PATH_LENGTH = 32;
byte path[MAX_PATH_LENGTH][2];
byte pathLen;


#include "MotorControl.h"

StepperMotor xMotor;
StepperMotor yMotor;
Axis x, y;
void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
  newCommand = false;newCommand = false;
  
  //sanity check
  if (xOffset + (boardWidth-1) * squareSize > xHome || yOffset + (boardHeight-1) * squareSize > yHome) {
    Serial.println("Error: square count is too big or board is too small");
    while (true) {}
  }

  xMotor = StepperMotor(xStep, xDir, xms1, xms2, xForward);
  yMotor = StepperMotor(yStep, yDir, yms1, yms2, yForward);
  xMotor.setMicrostepping(MICROSTEPPING);
  yMotor.setMicrostepping(MICROSTEPPING);
  x = Axis(xMotor, xHome, xLimSwitch);
  y = Axis(yMotor, yHome, yLimSwitch);
  x.setTarget(0);
  y.setTarget(0);
  
  pinMode(magnet, OUTPUT);
  digitalWrite(magnet, LOW);
  initializeBoard(1);
  
  Serial.println("ready");
}

void loop() {
  //check if a command has been recieved over serial and act on it
  receiveCommand();
  parseCommand();
  delay(100);

  //move
//  moveAxes();
}


// read from the serial pqueue until it is empty or a line terminateor is recieved
void receiveCommand() {
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
      msgLen = ndx + 1;
      ndx = 0;
      charIdx = 0;
      newCommand = true;
    }
  }
}

// check if a command has been recieved and act on it
void parseCommand() {
  if (newCommand) {
    Serial.print("received command: ");
    Serial.println(receivedChars);
    int n;

    //first character determines the type of command
    switch (receivedChars[charIdx++]) {
      case 'H': //home axes
        homeXY();
        break;
      case 'X': //go to x position
        n = getNum();
        if (n >= 0 and n <= xHome) {
          x.setTarget(n);
          moveAxes();
        } else {
          Serial.println("X position out of bounds");
        }
        break;
      case 'Y': //go to y position
        n = getNum();
        if (n >= 0 and n <= yHome) {
          y.setTarget(n);
          moveAxes();
        } else {
          Serial.println("Y position out of bounds");
        }
        break;

      case 'M': //move piece
        parseMove();
        break;
      case 'P': //play move
        parseAlgebraicNotation();
        break;
      case 'I': //initialize board state
        initializeBoard(getNum());
        break;
      case 'B':
        printBoardState();
        break;
      default:
        Serial.println("Unrecognized command");
        break;
    }
    ///
    newCommand = false;
  }
}


// read the next number from the available serial data
int getNum() {
  String numStr = "";
  char c;

  //skip whitespace
  while (charIdx < msgLen && receivedChars[charIdx] == ' ') {
    charIdx++;
  }

  // read the next set of consecutive digit characters
  while (charIdx < msgLen) {
    c = receivedChars[charIdx++];
    if (isDigit(c)) {
      numStr += c;
    } else {
      break;
    }
  }
  // no number available: return -1
  if (numStr.length() == 0) {
    return -1;
  }
  return (numStr.toInt());
}


void homeXY() {
  Serial.println("homing");

  //x
  if (x.homeAxis()) {
    Serial.println("X axis homed");
  } else {
    Serial.println("Error homing X axis");
    while (true) {}
  }
  delay(500);
  
  //y
  if (y.homeAxis()) {
    Serial.println("Y axis homed");
  } else {
    Serial.println("Error homing Y axis");
    while (true) {}
  }
  delay(500);
  
  x.setTarget(xHome - 20);
  y.setTarget(yHome - 20);
  moveAxes();
}

void parseAlgebraicNotation(){
  //Interpret a move in the form of long algebraic notation used by the Universal Chess Interface (e.g. e2e4)
  //TODO: handle castling and promotion
  int coords[4];
  int coordIdx = 0;

  //identify start and destination squares
  for(int i = 1; i < msgLen && coordIdx < 4; i ++){
    char c = receivedChars[i];
    if(coordIdx % 2 == 0){
      //file
      switch(c){
        case ' ': break;
        case 'a': coords[coordIdx++] = 0 + a1x; break;
        case 'b': coords[coordIdx++] = 1 + a1x; break;
        case 'c': coords[coordIdx++] = 2 + a1x; break;
        case 'd': coords[coordIdx++] = 3 + a1x; break;
        case 'e': coords[coordIdx++] = 4 + a1x; break;
        case 'f': coords[coordIdx++] = 5 + a1x; break;
        case 'g': coords[coordIdx++] = 6 + a1x; break;
        case 'h': coords[coordIdx++] = 7 + a1x; break;
        default: Serial.print("Invalid file: "); Serial.println(c); return;
      }
    }else{
      //rank
//      int r = c - '0';
      if('1' <= c && c <= '8'){
        coords[coordIdx++] = a1y + (c - '1');
      }else{
        Serial.print("Invalid rank: ");
        Serial.println(c);
        return;
      }
    }  
  }

  //check if the move is a capture
  char capturedPiece = boardState[coords[3]][coords[2]];
  if(capturedPiece != 0){
    Serial.println("Capturing");
    //move the captured piece off the board
    int storageCoords[2];
    getStorageSquare(capturedPiece, storageCoords);
    makeMove(coords[2], coords[3], storageCoords[0], storageCoords[1]);
  }

  //play the move
  makeMove(coords[0], coords[1], coords[2], coords[3]);
  
}

void getStorageSquare(char piece, int output[2]){
  //Find the proper storage location(off the board) for the specified piece
  //Search for a space that fits the piece type and is currently empty
  if(piece == PAWN){
    for(int i = a1y; i < a1y + 8; i ++){
      if(boardState[i][1] == 0){
        output[0] = 1;
        output[1] = i;
        return;
      }
    }
  }else if(piece == -PAWN){
    for(int i = a1y; i < a1y + 8; i ++){
      if(boardState[i][13] == 0){
        output[0] = 13;
        output[1] = i;
        return;
      }
    }
  }else{
    int f = (piece > 0)? 0 : 14;
    for(int i = a1y; i < a1y + 8; i++){
      if(EMPTY_POSITION[i][f] == piece && boardState[i][f] == 0){
        output[0] = f;
        output[1] = i;
      }
    }
  }
}

void parseMove() {
  int coords[4];
  
  for (int i = 0; i < 4; i ++) {
    coords[i] = getNum();
    if (coords[i] == -1) {
      Serial.println("Invalid move command");
      return;
    }
  }
  makeMove(coords[0], coords[1], coords[2], coords[3]);
}

//TODO: parse long algebreic notation?
//move a piece form one square to another and update the board state accordingly
void makeMove(int f0, int r0, int f1, int r1) {
  //find the best route to move the piece without disturbing the board
  findPath(f0, r0, f1, r1);
  
  Serial.print("Following path: ");
  for(int i = 0; i < pathLen; i ++){
    Serial.print("(");
    Serial.print(path[i][0]);
    Serial.print(", ");
    Serial.print(path[i][1]);
    Serial.print(") ");
  }
  Serial.println();

  goToSquare(f0, r0);
  delay(500);
  digitalWrite(magnet, HIGH);
  delay(500);
//  goToSquare(f1, r1);
  for(int i = 0; i < pathLen; i ++){
    //TODO: don't ramp down speed at every square
    goToSquare(path[i][0], path[i][1]);
  }
  delay(500);
  digitalWrite(magnet, LOW);


  //update board state
  boardState[r1][f1] = boardState[r0][f0];
  boardState[r0][f0] = 0;
}

//go to file f, square r (a1 = (0, 0))
void goToSquare(int f, int r) {
  Serial.print("Moving to square (");
  Serial.print(f);
  Serial.print(", ");
  Serial.print(r);
  Serial.println(")");
  
  int xSquare = f * squareSize + xOffset;
  int ySquare = r * squareSize + yOffset;
  if (xSquare < 0 || xSquare > xHome || ySquare < 0 || ySquare > yHome) {
    Serial.println("Square out of bounds");
    return;
  }

  x.setTarget(xSquare);
  y.setTarget(ySquare);
  moveAxes();
}

//move to the target position
void moveAxes(){
  if (x.isHomed && y.isHomed) {
    while (!(x.atTarget() && y.atTarget())) {
      x.updateAxis();
      y.updateAxis();
    }
  } else if (!(x.atTarget() && y.atTarget())) {
    Serial.println("no valid home position");
  }
}

// initialize the board's position memory
void initializeBoard(int state){
  switch(state){
    case 0:
      Serial.println("Empty position");
      memcpy(boardState, EMPTY_POSITION, sizeof(EMPTY_POSITION));
      break;
    case 1:
      Serial.println("Starting position");
      memcpy(boardState, STARTING_POSITION, sizeof(STARTING_POSITION));
      break;
    default:
      Serial.println("Invalid board state command");
  }
}

// find the shortest path betweeen two sqares, avoiding occupied squares
const int ORTH_COST = 1;
const int DIAG_COST = 1;
void findPath(int f0, int r0, int f1, int r1){
  int cost[boardHeight][boardWidth];
  int newCost[boardHeight][boardWidth];
  for(int i = 0; i < boardHeight; i++){
    for(int j = 0; j < boardWidth; j ++){
//      cost[i][j] = 32766;
//      newCost[i][j] = 32766;
      cost[i][j] = 30000;
      newCost[i][j] = 30000;
    }
  }
  cost[r0][f0] = 0;
  newCost[r0][f0] = 0;

  for(int iter = 0; iter < 100; iter++){
//    //print cost map
//    Serial.println("new cost:");
//    for(int y = 0; y < boardHeight; y++){
//      for(int x = 0; x < boardWidth; x++){
//        Serial.print(newCost[y][x]);
//        Serial.print(" ");
//      }
//      Serial.println();
//    }
    
    for(int i = 0; i < boardHeight; i ++){
      for(int j = 0; j < boardWidth; j ++){
        //consider orthogonal moves
        int c = cost[i][j] + ORTH_COST;
        if(i > 0 && boardState[i-1][j] == 0 && c < cost[i-1][j]){newCost[i-1][j] = c;}
        if(i < boardHeight-1 && boardState[i+1][j] == 0 && c < cost[i+1][j]){newCost[i+1][j] = c;}
        if(j > 0 && boardState[i][j-1] == 0 && c < cost[i][j-1]){newCost[i][j-1] = c;}
        if(j < boardWidth-1 && boardState[i][j+1] == 0 && c < cost[i][j+1]){newCost[i][j+1] = c;}

        //consider diagonal moves
        c = cost[i][j] + DIAG_COST;
        if(i > 0 && j > 0 && boardState[i-1][j-1] == 0 && c < cost[i-1][j-1]){newCost[i-1][j-1] = c;}
        if(i > 0 && j < boardWidth-1 && boardState[i-1][j+1] == 0 && c < cost[i-1][j+1]){newCost[i-1][j+1] = c;}
        if(i < boardHeight-1 && j > 0 && boardState[i+1][j-1] == 0 && c < cost[i+1][j-1]){newCost[i+1][j-1] = c;}
        if(i < boardHeight-1 && j < boardWidth-1 && boardState[i+1][j+1] == 0 && c < cost[i+1][j+1]){newCost[i+1][j+1] = c;}
      }
    }

    //check if the destination has been found
    //TODO: doesn't guaruntee minimum cost because if orthogonal & diagonal are different. Iterate until newCost doesn't change?
    if(newCost[r1][f1] < cost[r1][f1]){
      //trace the path from the destination square back to the starting square
//      Serial.println("tracing path");
      pathLen = newCost[r1][f1];
      int r = r1;
      int f = f1;

      for(int i = pathLen-1; i > 0; i--){
        path[i][0] = f;
        path[i][1] = r;
//        Serial.print(r);
//        Serial.print(" ");
//        Serial.println(f);

        //find the direction in which the cost function decreases
        int c = newCost[r][f];
        //check orthogonal moves
        if(r > 0 && newCost[r-1][f] < c){r--;}
        else if(r < boardWidth-1 && newCost[r+1][f] < c){r++;}
        else if(f > 0 && newCost[r][f-1] < c){f--;}
        else if(f < boardHeight-1 && newCost[r][f+1] < c){f++;}
        //check diagonal moves
        else if(r > 0 && f > 0 && newCost[r-1][f-1] < c){r--; f--;}
        else if(r < boardWidth-1 && f  > 0 && newCost[r+1][f-1] < c){r++; f--;}
        else if(r > 0 && f < boardHeight-1 && newCost[r-1][f+1] < c){r--; f++;}
        else if(r > boardWidth-1 && f < boardHeight-1 && newCost[r+1][f+1] < c){r++; f++;}
        else{
          //something has gone wrong
          Serial.println("Failed to find a path");
          pathLen = 0;
          return;
        }
      }
      path[0][0] = f;
      path[0][1] = r;
      return;
    }

    //update costs of all squares at once
    memcpy(cost, newCost, sizeof(cost));
  }
  Serial.println("Failed to find a path");
  pathLen = 0;
}

//print an ASCII representation of the board state
void printBoardState(){
  for(int i = 0; i < boardHeight; i ++){
    for(int j = 0; j < boardWidth; j ++){
      switch(boardState[boardHeight - i - 1][j]){
        case  PAWN: Serial.print('P'); break;
        case -PAWN: Serial.print('p'); break;
        case  KNIGHT: Serial.print('N'); break;
        case -KNIGHT: Serial.print('n'); break;
        case  BISHOP: Serial.print('B'); break;
        case -BISHOP: Serial.print('b'); break;
        case  ROOK: Serial.print('R'); break;
        case -ROOK: Serial.print('r'); break;
        case  QUEEN: Serial.print('Q'); break;
        case -QUEEN: Serial.print('q'); break;
        case  KING: Serial.print('K'); break;
        case -KING: Serial.print('k'); break;
//        default: Serial.print('X');
        default: Serial.print('.');
      }
      Serial.print(' ');
    }
    Serial.println();
  }
}
