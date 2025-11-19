// find the shortest path betweeen two sqares, avoiding occupied squares
void findPath(byte x0, byte y0, byte x1, byte y1){
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

  unsigned int finalized[boardHeight] = {0}; // when the shortest path to a square is known, set the corresponding bit to 1
  finalized[y0] |= (0b1 << x0);

  for(byte i = 0; i < boardHeight; i++){
    for(byte j = 0; j < boardWidth; j ++){
      cost[i][j] = MAX_UINT;
      pathDx[i][j] = 0;
      pathDy[i][j] = 0;
    }
  }
  cost[y0][x0] = 0;
  exploreNode(x0, y0);
  // newCost[y0][x0] = 0;

  while(true){
    // find the non-finalized node with the lowest cost estimate
    // the cost estimate cannot get any lower, so we can finalize it
    byte nextX = 255;
    byte nextY = 255;
    unsigned int nextCost = MAX_UINT;

    for(byte y = 0; y < boardHeight; y ++){
      for(byte x = 0; x < boardWidth; x ++){
        bool isFinal = (finalized[y] >> x) & 0b1 == 0b1;
        if(!isFinal && cost[y][x] < nextCost){
          nextX = x;
          nextY = y;
          nextCost = cost[y][x];
        }
      }
    }

    if(nextX == 255){
      // no unfinalized node: the entire graph has been searched
        pathLen = 0;
        Serial.print(F("Failed to find a path from "));
        printCoordinates(x0, y0);
        Serial.print(F(" to "));
        printCoordinates(x1, y1);
        Serial.println();

        //print final cost map
        Serial.println(F("cost:"));
        for(int y = 0; y < boardHeight; y++){
          for(int x = 0; x < boardWidth; x++){
            Serial.print(cost[y][x]);
            Serial.print(F("\t"));
          }
          Serial.println();
        }
        return;
    }

    // check if the node to finalize is the destination
    if(nextX == x1 && nextY == y1){
      tracePath(x1, y1);
      return;
    }

    // mark the node as finalized and update its neighbors
    finalized[nextY] |= (0b1 << nextX);
    // can't move to or from an occupied square
    if(boardState[nextY][nextX] == 0){
      exploreNode(nextX, nextY);
    } 
  }
}

// Update the cost estimate of all nodes adjacent to (x, y)
void exploreNode(byte x, byte y){
  checkOrthMove(x, y, 1, 0);
  checkOrthMove(x, y, -1, 0);
  checkOrthMove(x, y, 0, 1);
  checkOrthMove(x, y, 0, -1);

  checkDiagMove(x, y, 1, 1);
  checkDiagMove(x, y, -1, 1);
  checkDiagMove(x, y, 1, -1);
  checkDiagMove(x, y, -1, -1);

  checkKnightMove(x, y, 2, 1);
  checkKnightMove(x, y, -2, 1);
  checkKnightMove(x, y, 2, -1);
  checkKnightMove(x, y, -2, -1);
  checkKnightMove(x, y, 1, 2);
  checkKnightMove(x, y, -1, 2);
  checkKnightMove(x, y, 1, -2);
  checkKnightMove(x, y, -1, -2);


}

// evaluate whether the orthogonal from (x, y) to (x+dx, y+dy) results in a lower total path cost
void checkOrthMove(byte x, byte y, char dx, char dy){
  // validity checks
  if(!(0 <= x+dx && x+dx < boardWidth && 0 <= y+dy && y+dy < boardHeight)){return;}
  if(boardState[y+dy][x+dx] != 0){return;}

  //update the cost map if it results in a lower cost estimate
  unsigned int c = cost[y][x] + ORTH_COST;
  if(dx != pathDx[y][x] || dy != pathDy[y][x]){c += DIRECTION_CHANGE_PENALTY;}
  if(cost[y+dy][x+dx] > c){
    cost[y+dy][x+dx] = c;
    pathDx[y+dy][x+dx] = dx;
    pathDy[y+dy][x+dx] = dy;
  }
}

void checkDiagMove(byte x, byte y, char dx, char dy){
  // validity checks
  if(!(0 <= x+dx && x+dx < boardWidth && 0 <= y+dy && y+dy < boardHeight)){return;}
  if(boardState[y+dy][x+dx] != 0){return;}

  unsigned int c = cost[y][x] + DIAG_COST;
  if(dx != pathDx[y][x] || dy != pathDy[y][x]){c += DIRECTION_CHANGE_PENALTY;}
  // add a penalty for passing close to an occupied square
  if(boardState[y][x+dx] != 0){
    c += CORNER_CUT_PENALTY;
  }
  if(boardState[y+dy][x] != 0){
    c += CORNER_CUT_PENALTY;
  }

  //update the cost map if it results in a lower cost estimate
  if(cost[y+dy][x+dx] > c){
    cost[y+dy][x+dx] = c;
    pathDx[y+dy][x+dx] = dx;
    pathDy[y+dy][x+dx] = dy;
  }
}

void checkKnightMove(byte x, byte y, char dx, char dy){
  // validity checks
  if(!(0 <= x+dx && x+dx < boardWidth && 0 <= y+dy && y+dy < boardHeight)){return;}
  if(boardState[y+dy][x+dx] != 0){return;}
  //collision checks
  if(abs(dx) ==2){
    if(boardState[y+dy][x+(dx/2)] != 0 || boardState[y][x+(dx/2)] != 0){return;}
  }else{
    if(boardState[y+(dy/2)][x+dx] != 0 || boardState[y+(dy/2)][x] != 0){return;}
  }

  //update the cost map if it results in a lower cost estimate
  unsigned int c = cost[y][x] + KNIGHT_COST;
  if(dx != pathDx[y][x] || dy != pathDy[y][x]){c += DIRECTION_CHANGE_PENALTY;}
  if(cost[y+dy][x+dx] > c){
    cost[y+dy][x+dx] = c;
    pathDx[y+dy][x+dx] = dx;
    pathDy[y+dy][x+dx] = dy;
  }
}


// trace the path from the destination square back to the starting square
// call only after finding the cost map
void tracePath(byte x, byte y){
//  Serial.println(F("tracing path"));
  pathLen = 0;
  // path[0][0] = x;
  // path[0][1] = y;

  for(int i = 0; i < 1000; i ++){
    path[pathLen][0] = x;
    path[pathLen][1] = y;
    pathLen++;

    // take a step back along the optimal path
    char dx = pathDx[y][x];
    char dy = pathDy[y][x];
    // the starting square has no parent
    if(dx == 0 && dy == 0){
      Serial.print("pathLen: ");
      Serial.println(pathLen);
      reversePath();
      return;
    }
    x -= dx;
    y -= dy;
  }
  Serial.println(F("Failed to trace path"));
}

// reverse the order of the path array
void reversePath(){
  for(byte i = 0; true; i++){
    byte j = pathLen - 1 - i;
    if(i >= j){break;}

    byte tmp0 = path[i][0];
    byte tmp1 = path[i][1];
    path[i][0] = path[j][0];
    path[i][1] = path[j][1];
    path[j][0] = tmp0;
    path[j][1] = tmp1;
  }
}
