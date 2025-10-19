//pin definitions
#define xStep 5
#define xDir 4
#define yStep 7
#define yDir 6
#define xms1 10
#define xms2 9
#define yms1 12
#define yms2 11
#define xLimSwitch 2
#define yLimSwitch 3
#define magnet 8

#define xForward LOW
#define yForward LOW
#define MICROSTEPPING 4

#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

//position of limit switches (steps, 0 = minimum)
#define xHome 2100
#define yHome 1290

//(1 step = 0.2mm)
#define squareSize 140
//position of the center of the bottom-right square
#define xOffset 100
#define yOffset 15
//square coordinates of a1
#define a1x 4
#define a1y 1

#define boardWidth 15
#define boardHeight 10
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
  {0, ROOK, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -ROOK},
  {0, KNIGHT, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KNIGHT},
  {0, BISHOP, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -BISHOP},
  {0, QUEEN, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -QUEEN},
  {0, KING, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KING},
  {0, BISHOP, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -BISHOP},
  {0, KNIGHT, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KNIGHT},
  {0, ROOK, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -ROOK},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
//const char EMPTY_POSITION[boardHeight][boardWidth] = {
//  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//  {ROOK, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -ROOK},
//  {KNIGHT, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KNIGHT},
//  {BISHOP, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -BISHOP},
//  {KING, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KING},
//  {QUEEN, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -QUEEN},
//  {BISHOP, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -BISHOP},
//  {KNIGHT, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -KNIGHT},
//  {ROOK, PAWN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -PAWN, -ROOK},
//  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
//};

//Square cost grids for move planning
byte cost[boardHeight][boardWidth];
byte newCost[boardHeight][boardWidth];
//movement costs for move planning
#define ORTH_COST 1
#define DIAG_COST 1

unsigned long t;
unsigned long xLastTime, yLastTime;

//for serial communication
#define endMarker ';'
#define numChars 16
char receivedChars[numChars];
byte charIdx; //indicates the next recieved character to parse
byte msgLen;
boolean newCommand;
//for move commands following a path
const byte MAX_PATH_LENGTH = 32;
byte path[MAX_PATH_LENGTH][2];
byte pathLen;

byte storageCoords[2];


#include "MotorControl.h"

StepperMotor xMotor;
StepperMotor yMotor;
Axis xAxis, yAxis;
void setup() {
  Serial.begin(9600);
  Serial.println(F("Initializing..."));
  newCommand = false;
  
  //sanity check
  if (xOffset + (boardWidth-1) * squareSize > xHome || yOffset + (boardHeight-1) * squareSize > yHome) {
    Serial.println(F("Error: square count is too big or board is too small"));
    while (true) {}
  }

  xMotor = StepperMotor(xStep, xDir, xms1, xms2, xForward);
  yMotor = StepperMotor(yStep, yDir, yms1, yms2, yForward);
  xMotor.setMicrostepping(MICROSTEPPING);
  yMotor.setMicrostepping(MICROSTEPPING);
  xAxis = Axis(xMotor, xHome, xLimSwitch);
  yAxis = Axis(yMotor, yHome, yLimSwitch);
  xAxis.setTarget(0);
  yAxis.setTarget(0);
  
  pinMode(magnet, OUTPUT);
  digitalWrite(magnet, LOW);
  initializeBoard(1);
  
  Serial.println(F("ready"));
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
    Serial.print(F("received command: "));
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
          xAxis.setTarget(n);
          moveAxes();
        } else {
          Serial.println(F("X position out of bounds"));
        }
        break;
      case 'Y': //go to y position
        n = getNum();
        if (n >= 0 and n <= yHome) {
          yAxis.setTarget(n);
          moveAxes();
        } else {
          Serial.println(F("Y position out of bounds"));
        }
        break;

      case 'M': //move piece
        parseMove();
        break;
      case 'P': //play move
        parseAlgebraicNotation();
        break;
      case 'A':
        alignPieces();
        break;
      case 'S':
        setupBoard(getNum());
        break;
      case 'I': //initialize board state
        initializeBoard(getNum());
        break;
      case 'B':
        printBoardState();
        break;
      case 'D':
        printDimensions();
        break;
      default:
        Serial.println(F("Unrecognized command"));
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
  Serial.println(F("homing"));
  //x
  if (xAxis.homeAxis()) {
    Serial.println(F("X axis homed"));
  } else {
    Serial.println(F("Error homing X axis"));
    while (true) {}
  }
  delay(500);
  
  //y
  if (yAxis.homeAxis()) {
    Serial.println(F("Y axis homed"));
  } else {
    Serial.println(F("Error homing Y axis"));
    while (true) {}
  }
  delay(500);
  
  xAxis.setTarget(xHome - 20);
  yAxis.setTarget(yHome - 20);
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
        default: Serial.print(F("Invalid file: ")); Serial.println(c); return;
      }
    }else{
      //rank
//      int r = c - '0';
      if('1' <= c && c <= '8'){
        coords[coordIdx++] = a1y + (c - '1');
      }else{
        Serial.print(F("Invalid rank: "));
        Serial.println(c);
        return;
      }
    }  
  }

  //check if the move is a capture
  char capturedPiece = boardState[coords[3]][coords[2]];
  if(capturedPiece != 0){
    Serial.print(F("Capturing at "));
    printCoordinates(coords[2], coords[3]);
    Serial.println();
    //move the captured piece off the board
    getStorageSquare(capturedPiece);
    makeMove(coords[2], coords[3], storageCoords[0], storageCoords[1]);
  }

  //play the move
  makeMove(coords[0], coords[1], coords[2], coords[3]);
  
}

void getStorageSquare(char piece){
  //Find the proper storage location(off the board) for the specified piece
  //Search for a space that fits the piece type and is currently empty
  //Put the result in the storageCoords array
  if(piece == PAWN || piece == -PAWN){
    byte x = (piece == PAWN)? 2 : 13;
    for(byte i = a1y; i < a1y + 7; i ++){
      if(boardState[i][1] == 0){
        storageCoords[0] = x;
        storageCoords[1] = i;
        return;
      }
    }
  }else{
    byte x = (piece > 0)? 1 : 14;
    for(byte i = a1y; i < a1y + 8; i++){
      if(EMPTY_POSITION[i][x] == piece && boardState[i][x] == 0){
        storageCoords[0] = x;
        storageCoords[1] = i;
      }
    }
  }
}

void parseMove() {
  int coords[4];
  for (int i = 0; i < 4; i ++) {
    coords[i] = getNum();
    if (coords[i] == -1) {
      Serial.println(F("Invalid move command"));
      return;
    }
  }
  makeMove(coords[0], coords[1], coords[2], coords[3]);
}

//move a piece form one square to another and update the board state accordingly
void makeMove(int x0, int y0, int x1, int y1) {
  //find the best route to move the piece without disturbing the board
  findPath(x0, y0, x1, y1);
  
  Serial.print(F("Following path: "));
  for(int i = 0; i < pathLen; i ++){
    printCoordinates(path[i][0], path[i][1]);
  }
  Serial.println();
  if(pathLen == 0){return;}

//  goToSquare(f0, r0);
  goToSquare(path[0][0], path[0][1]);
//  delay(500);
  digitalWrite(magnet, HIGH);
  delay(200);
  //follow the path
  for(int i = 1; i < pathLen; i ++){
    //skip squares that lie on the same line to avoid unnecessary stopping and starting
    char dx = path[i][0] - path[i-1][0];
    char dy = path[i][1] - path[i-1][1];
    bool skipSquare = (i < pathLen-1) && (path[i+1][0] - path[i][0] == dx) && (path[i+1][1] - path[i][1] == dy);
    if(!skipSquare){
      goToSquare(path[i][0], path[i][1]);
    }
  }
  
  delay(200);
  digitalWrite(magnet, LOW);

  //update board state
  boardState[y1][x1] = boardState[y0][x0];
  boardState[y0][x0] = 0;
}

//go to file x, rank y (a1 = (0, 0))
void goToSquare(int x, int y) {
  Serial.print(F("Moving to square "));
  printCoordinates(x, y);
  Serial.println();
  
  int xSquare = x * squareSize + xOffset;
  int ySquare = y * squareSize + yOffset;
  if (xSquare < 0 || xSquare > xHome || ySquare < 0 || ySquare > yHome) {
    Serial.println(F("Square out of bounds"));
    return;
  }

  xAxis.setTarget(xSquare);
  yAxis.setTarget(ySquare);
  moveAxes();
}

//move to the target position
void moveAxes(){
  if (xAxis.isHomed && yAxis.isHomed) {
    while (!(xAxis.atTarget() && yAxis.atTarget())) {
      xAxis.updateAxis();
      yAxis.updateAxis();
    }
  } else if (!(xAxis.atTarget() && yAxis.atTarget())) {
    Serial.println(F("no valid home position"));
  }
}

//Straighten out the pieces by going to every square with a piece on it and powering the magnet
void alignPieces(){
  byte x;
  for(byte y = 0; y < boardHeight; y++){
    for(byte i = 0; i < boardWidth; i++){
      x = (y % 2 == 0)? i : boardWidth - i - 1;
      if(boardState[y][x] != 0){
        goToSquare(x, y);
        digitalWrite(magnet, HIGH);
        delay(300);
        digitalWrite(magnet, LOW);
      }
    }
  }
}

//Set up the pieces in the desired position
void setupBoard(int state){
  switch(state){
    case 0:
      Serial.println(F("Setting up empty position"));
      setPosition(EMPTY_POSITION);
      break;
    case 1:
      Serial.println(F("Setting up starting position"));
      setPosition(STARTING_POSITION);
      break;
    default:
      Serial.println(F("Invalid board setup command"));
      return;
  }
}

//TODO: it is possible to have a configuration where this tries to move a trapped piece and fails
//TODO: more efficient order
void setPosition(const char desiredState[boardHeight][boardWidth]){
  //For each piece in the desired position, find where it is and get it
  for(byte y = 0; y < boardHeight; y++){
    for(byte x = 0; x < boardWidth; x++){
      char desiredPiece = desiredState[y][x];
      //check if there should be a piece on this square and isn't
      if(desiredPiece != 0 && desiredPiece != boardState[y][x]){
        //if there is already a piece on this square, try moving it aside
        if(boardState[y][x] != 0){
          if(x > 0 && boardState[y][x-1] == 0){makeMove(x, y, x-1, y);}
          else if(x < boardWidth && boardState[y][x+1] == 0){makeMove(x, y, x+1, y);}
          else if(y > 0 && boardState[y-1][x] == 0){makeMove(x, y, x, y-1);}
          else if(y < boardHeight && boardState[y+1][x] == 0){makeMove(x, y, x, y+1);}
          else if(x > 0 && y > 0 && boardState[y-1][x-1] == 0){makeMove(x, y, x-1, y-1);}
          else if(x > 0 && y < boardHeight && boardState[y+1][x-1] == 0){makeMove(x, y, x-1, y+1);}
          else if(x < boardWidth && y > 0 && boardState[y-1][x+1] == 0){makeMove(x, y, x+1, y-1);}
          else if(x < boardWidth && y < boardHeight && boardState[y+1][x+1] == 0){makeMove(x, y, x+1, y+1);}
        }
        
        //search for the desired piece
        for(byte ySource = 0; ySource < boardHeight; ySource++){
          for(byte xSource = 0; xSource < boardWidth; xSource++){
            //take the desired piece from where it is, unless it belongs there
            if(boardState[ySource][xSource] == desiredPiece && boardState[ySource][xSource] != desiredState[ySource][xSource]){
              makeMove(xSource, ySource, x, y);
              //break out of the nested loop
              ySource = boardHeight;
              break;
            }
          }
        }
        Serial.println(F("Failed to find piece"));
      }
    }
  }
}

// initialize the board's position memory
void initializeBoard(int state){
  switch(state){
    case 0:
      Serial.println(F("Empty position"));
      memcpy(boardState, EMPTY_POSITION, sizeof(EMPTY_POSITION));
      break;
    case 1:
      Serial.println(F("Starting position"));
      memcpy(boardState, STARTING_POSITION, sizeof(STARTING_POSITION));
      break;
    default:
      Serial.println(F("Invalid board state command"));
  }
}

//TODO: prioritize straightaways, deprioritize paths that pass close to other pieces (diagonals especially)
// find the shortest path betweeen two sqares, avoiding occupied squares
void findPath(int x0, int y0, int x1, int y1){
  if(x0 == x1 && y0 == y1){
    Serial.println(F("null move"));
    pathLen = 0;
    return;
  }
  if(boardState[y1][x1] != 0){
    Serial.println(F("Error in path finding: destination square "));
    printCoordinates(x1, y1);
    Serial.println(F(" is occupied"));
    pathLen = 0;
    return;
  }
  
  for(int i = 0; i < boardHeight; i++){
    for(int j = 0; j < boardWidth; j ++){
//      cost[i][j] = 32766;
//      newCost[i][j] = 32766;
//      cost[i][j] = 30000;
//      newCost[i][j] = 30000;
      cost[i][j] = 255;
      newCost[i][j] = 255;
    }
  }
  cost[y0][x0] = 0;
  newCost[y0][x0] = 0;

  for(int iter = 0; iter < 100; iter++){
//    //print cost map
//    Serial.println(F("new cost:"));
//    for(int y = 0; y < boardHeight; y++){
//      for(int x = 0; x < boardWidth; x++){
//        Serial.print(newCost[y][x]);
//        Serial.print(F("\t"));
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
    //TODO: doesn't guaruntee minimum cost unless orthogonal & diagonal are teh same cost. Iterate until newCost doesn't change?
    if(newCost[y1][x1] < cost[y1][x1]){
      //trace the path from the destination square back to the starting square
//      Serial.println(F("tracing path"));
      pathLen = newCost[y1][x1] + 1;
      int y = y1;
      int x = x1;

      for(int i = pathLen-1; i > 0; i--){
        path[i][0] = x;
        path[i][1] = y;

        //find the direction in which the cost function decreases
        int c = newCost[y][x];
        //check orthogonal moves
        if(y > 0 && newCost[y-1][x] < c){y--;}
        else if(y < boardHeight-1 && newCost[y+1][x] < c){y++;}
        else if(x > 0 && newCost[y][x-1] < c){x--;}
        else if(x < boardWidth-1 && newCost[y][x+1] < c){x++;}
        //check diagonal moves
        else if(y > 0 && x > 0 && newCost[y-1][x-1] < c){y--; x--;}
        else if(y < boardHeight-1 && x  > 0 && newCost[y+1][x-1] < c){y++; x--;}
        else if(y > 0 && x < boardWidth-1 && newCost[y-1][x+1] < c){y--; x++;}
        else if(y < boardHeight-1 && x < boardWidth-1 && newCost[y+1][x+1] < c){y++; x++;}
        else{
          //something has gone wrong
          Serial.print(F("Error in backtracing path: stuck at "));
          printCoordinates(x, y);
          //print final cost map
          Serial.println(F("cost:"));
          for(int j = 0; j < boardHeight; j++){
            for(int k = 0; k < boardWidth; k++){
              Serial.print(newCost[j][k]);
              Serial.print(F("\t"));
            }
            Serial.println();
          }
          pathLen = 0;
          return;
        }
      }
      path[0][0] = x;
      path[0][1] = y;
      return;
    }

    //update costs of all squares at once
    memcpy(cost, newCost, sizeof(cost));
  }
  // if the loop completes: no path was found
  pathLen = 0;
  Serial.print(F("Failed to find a path from "));
  printCoordinates(x0, y0);
  Serial.print(F(" to "));
  printCoordinates(x1, y1);
  Serial.println();
  
  //print final cost map
  Serial.println(F("cost:"));
  for(int x = 0; x < boardWidth; x++){
    for(int y = 0; y < boardHeight; y++){
      Serial.print(newCost[y][x]);
      Serial.print(F("\t"));
    }
    Serial.println();
  }
}

//print an ASCII representation of the board state
void printBoardState(){
  for(int i = 0; i < boardHeight; i ++){
    for(int j = 0; j < boardWidth; j ++){
      switch(boardState[boardHeight - i - 1][j]){
        case  PAWN: Serial.print(F("P")); break;
        case -PAWN: Serial.print(F("p")); break;
        case  KNIGHT: Serial.print(F("N")); break;
        case -KNIGHT: Serial.print(F("n")); break;
        case  BISHOP: Serial.print(F("B")); break;
        case -BISHOP: Serial.print(F("b")); break;
        case  ROOK: Serial.print(F("R")); break;
        case -ROOK: Serial.print(F("r")); break;
        case  QUEEN: Serial.print(F("Q")); break;
        case -QUEEN: Serial.print(F("q")); break;
        case  KING: Serial.print(F("K")); break;
        case -KING: Serial.print(F("k")); break;
        default: 
          if(a1x <= j && j <= a1x+7 && a1y <= i && i <= a1y+7){
            Serial.print(F("."));
          }else{
            Serial.print(F("-"));
          }
      }
      Serial.print(F(" "));
    }
    Serial.println();
  }
}

//resport the size of the usable area, in both moror steps and squares
void printDimensions(){
  Serial.print(F("Board dimensions:\n\t"));
  Serial.print(xHome);
  Serial.print(F(" x "));
  Serial.print(yHome);
  Serial.print(F(" steps\n\t"));
  Serial.print(xHome * 0.2);
  Serial.print(F(" x "));
  Serial.print(yHome * 0.2);
  Serial.print(F(" mm\n\t"));
  Serial.print(boardWidth);
  Serial.print(F(" x "));
  Serial.print(boardHeight);
  Serial.println(F(" squares"));

  Serial.print(F("Square width: "));
  Serial.print(squareSize);
  Serial.print(F(" steps ("));
  Serial.print(squareSize * 0.2);
  Serial.println(F(" mm)"));
  
  Serial.print(F("a1 is square "));
  printCoordinates(a1x, a1y);
  Serial.println();
}

void printCoordinates(int x, int y){
  Serial.print(F("("));
  Serial.print(x);
  Serial.print(F(", "));
  Serial.print(y);
  Serial.print(F(")"));
}
