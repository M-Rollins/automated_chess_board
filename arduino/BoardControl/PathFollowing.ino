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

void executePath(){
  //determine desired velocities at each point on the path (compromise between the incoming and outgoing segment)
  float vx[pathLen] = {0};
  float vy[pathLen] = {0};
  for(int i = 1; i < pathLen - 1; i ++){
    char dx1 = path[i][0] - path[i-1][0];
    char dx2 = path[i+1][0] - path[i][0];
    char dy1 = path[i][1] - path[i-1][1];
    char dy2 = path[i+1][1] - path[i][1];

    float vx1, vx2, vy1, vy2;
    if(abs(dx1) > abs(dy1)){
      vx1 = sgn(dx1) * xAxis.V_MAX;
      vy1 = (vx1 * dy1) / dx1;
    }else{
      vy1 = sgn(dy1) * yAxis.V_MAX;
      vx1 = (vy1 * dx1) / dy1;
    }
    if(abs(dx2) > abs(dy2)){
      vx2 = sgn(dx2) * xAxis.V_MAX;
      vy2 = (vx2 * dy2) / dx2;
    }else{
      vy2 = sgn(dy2) * yAxis.V_MAX;
      vx2 = (vy2 * dx2) / dy2;
    }

    vx[i] = (vx1 + vx2) / 2;
    vy[i] = (vy1 + vy2) / 2;
  }

  //follow the path
  for(int i = 1; i < pathLen; i ++){
    // start and end positions of the segmeent
    int x0 = path[i-1][0] * squareSize + xOffset;
    int x1 = path[i][0] * squareSize + xOffset;
    int y0 = path[i-1][1] * squareSize + yOffset;
    int y1 = path[i][1] * squareSize + yOffset;
    int dx = x1 - x0;
    int dy = y1 - y0;
    int sx = sgn(dx);
    int sy = sgn(dy);
    
    //start and end velocities
    float v0x = vx[i-1];
    float vfx = vx[i];
    float v0y = vy[i-1];
    float vfy = vy[i];

    //check which axis needs more time to reach its position
    float xTimes[3], yTimes[3];
    getTimeSplit(sx*dx, xAxis.ACCEL, xAxis.V_MAX, sx*v0x, sx*vfx, xTimes);
    getTimeSplit(sy*dy, yAxis.ACCEL, yAxis.V_MAX, sy*v0y, sy*vfy, yTimes);

    //The slower axis uses the shortest-time path. The other follows a cubic spline that finishes at the same time
    if(xTimes[2] > yTimes[2]){
      xAxis.followRampCycle(v0x, sx*xAxis.ACCEL, xTimes[0], xTimes[1], xTimes[2]);
      float c[3];
      getCubicSplineParams(dy, v0y, vfy, xTimes[2], c);
      yAxis.followSpline(c[0], c[1], c[2], xTimes[2]);
      //update node velocity estimate
      vx[i] = v0x + sx*xAxis.ACCEL * (xTimes[0] - (xTimes[2]-xTimes[1]));
    }else{
      yAxis.followRampCycle(v0y, sy*yAxis.ACCEL, yTimes[0], yTimes[1], yTimes[2]);
      float c[3];
      getCubicSplineParams(dx, v0x, vfx, yTimes[2], c);
      xAxis.followSpline(c[0], c[1], c[2], yTimes[2]);
      vy[i] = v0y + sy*yAxis.ACCEL * (yTimes[0] - (yTimes[2]-yTimes[1]));
    }
    moveAxes();
  }
}

/*Calculate key times (sop accelerating, start decelerating, finish) to get from x=0 to dx as quickly as possible*/
float* getTimeSplit(int dx, float a, float v_max, float v0, float vf, float* output){
  float t1 = (v_max - v0) / a;
  float t2, t3;

  //check if there is time to get to max speed
  float dt3 = (v_max - vf) / a;
  if (v0 * t1 + 0.5 * a * (t1*t1 - dt3*dt3) + v_max * dt3 > dx){
      //if ramping to full speed, then back down would overshoot, we need to stop short of max speed
      float t_asym = (vf - v0) / a;

      //check if there is actually time to achieve the target final velocity
      if((vf > v0) && (v0 * t_asym + 0.5 * a * t_asym*t_asym > dx)){
          //if not, accelerate from start to finish
          t1 = (-v0 + sqrt(v0*v0 + 2 * a * dx)) / a;
          t2 = t1;
          t3 = t1;
      }else if((vf < v0) && (v0 * (-t_asym) - 0.5 * a * t_asym*t_asym > dx)){
          t1 = 0;
          t2 = 0;
          t3 = -(-v0 + sqrt(v0*v0 - 2 * a * dx)) / a;
      }else{
        //quadratic formula
        //aa = a
        float bb = -a * t_asym + v0 + vf;
        float cc = 0.5 * a * t_asym*t_asym - vf * t_asym - dx;
        
        t1 = (-bb + sqrt(bb*bb - 4*a*cc)) / (2 * a);
        t2 = t1;
        t3 = 2 * t1 - t_asym;
      }
  
  }else{
      float dt2 = (dx - v0*t1 - v_max*dt3 + 0.5*a*(dt3*dt3 - t1*t1)) / v_max;
      
      t2 = t1 + dt2;
      t3 = t2 + dt3;
  }
  //0-t1: acceleration, t1-t2: constant speed, t2-t3: deceleration
  output[0] = t1;
  output[1] = t2;
  output[2] = t3;
}

/*Calculate a cubic trajectory(x(t) = at^3 + bt^2 + ct + 0) such that: at t=0, x=0, v=v0; at t=tf, x=xf, v=vf*/
float* getCubicSplineParams(float xf, float v0, float vf, float tf, float* output){
  //c = v0
  float b = (-2*v0 - vf) / tf + 3*xf / (tf*tf);
  float a = (v0 + vf) / (tf*tf) - 2*xf / (tf*tf*tf);

  output[0] = a;
  output[1] = b;
  output[2] = v0;
}
