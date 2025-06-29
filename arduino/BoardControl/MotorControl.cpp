/*
 * motor_control.cpp - classes for cnc control of stepper-driven motion axes
 */
#include "Arduino.h"
#include "MotorControl.h"

const int StepperMotor::PULSE_WIDTH_MICROSECONDS = 5;
const float Axis::V_MIN = 50;  // full steps per second
const float Axis::V_MAX = 1000;
const float Axis::HOME_SPEED = 400;
const float Axis::ACCEL = 1000;  //full steps/s^2

//=======================================================
StepperMotor::StepperMotor(){}
StepperMotor::StepperMotor(int s, int d, int msPin1, int msPin2, boolean f){
  stepPin = s;
  dirPin = d;
  ms1 = msPin1;
  ms2 = msPin2;
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(ms1, OUTPUT);
  pinMode(ms2, OUTPUT);
  setMicrostepping(1);
  
  forward = f;
  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, forward);
}

boolean StepperMotor::setMicrostepping(int steps){
  switch(steps){
    case 1:
      digitalWrite(ms1, LOW);
      digitalWrite(ms2, LOW);
      return true;
    case 2:
      digitalWrite(ms1, HIGH);
      digitalWrite(ms2, LOW);
      return true;
    case 4:
      digitalWrite(ms1, LOW);
      digitalWrite(ms2, HIGH);
      return true;
    case 8:
      digitalWrite(ms1, HIGH);
      digitalWrite(ms2, HIGH);
      return true;
    default:
      Serial.println("Invalid microstep resolution. Only values of 1, 2, 4, and 8 are valid.");
      return false;    
  }
}

void StepperMotor::step(boolean d){
  digitalWrite(dirPin, d == forward);
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(PULSE_WIDTH_MICROSECONDS);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(PULSE_WIDTH_MICROSECONDS);
}

//=======================================================
Axis::Axis(){}
Axis::Axis(StepperMotor m, int lp, int switchPin){
  motor = m;
  limitPos = lp;
  safetyLimit = limitPos + 1000;
  limSwitch = switchPin;
  pos = 0;
  target = 0;
  v = 0;
  isHomed = false;
  pinMode(limSwitch, INPUT_PULLUP);
  setMicrostepping(1);
}

void Axis::setMicrostepping(int steps){
  if(motor.setMicrostepping(steps)){
    microstepping = steps;
  }else{
    Serial.println("error setting microstepping resolution");
  }
}

void Axis::setTarget(int t){
  target = t;
}

boolean Axis::atTarget(){
//  return pos == target && v == 0;
  return pos == target;
}

boolean Axis::homeAxis(){
  float lastTime = micros();
  stepDelay = 1000000 / (HOME_SPEED * microstepping);

  //move the axis until reaching the limit switch
  while(!digitalRead(limSwitch)){
    unsigned long t = micros();
    if(t - lastTime > stepDelay){
      //update the time, but avoid sending many steps in quick succession if there is a delay
      while(t - lastTime >= stepDelay) {lastTime += stepDelay;};
      motor.step(true);
      pos += 1./microstepping;

      //stop the program if the limit switch is not detected
      if(pos >= safetyLimit){
        isHomed = false;
        return false;
      }
    }
  }
  //recognize the home position
  pos = limitPos;
  target = pos;
  isHomed = true;
  return true;
}


void Axis::updateAxis(){
  //don't move without knowing where the axis is
  if(!isHomed || pos == target){
    return;
  }
  
//  //stop moving when the target is reached
//  if(pos == target){
//    v = 0;
//    return;
//  }else 
  if(v == 0){
    // when the motor is stopped and gets a new target position, start moving
    v = V_MIN;
    rampUp = true;
    lastTime = micros();
    return;
  }
  
  //move the motor
  stepDelay = 1000000 / (v * microstepping);

  //TODO: overflow protection? (rolls over every ~1hr)
  unsigned long t = micros();
  if(t - lastTime >= stepDelay){
    //update the time, but avoid sending many steps in quick succession if there is a delay
    while(t - lastTime >= stepDelay) {lastTime += stepDelay;};
    if(pos < target){
      motor.step(true);
      pos += 1./microstepping;
    }else if (pos > target){
      motor.step(false);
      pos -= 1./microstepping;
    }
    //check the limit swicth after each step
    if(digitalRead(limSwitch)){
      pos = limitPos;
    }
    
    //accelerate up to max speed
    if(rampUp){
      v += ACCEL * stepDelay/1000000.;
      if(v >= V_MAX) {
        v = V_MAX;
        rampUp = false;
      }
    }
    // decelerate down to minimum speed as the destination approaches
    float stoppingDist = 1. / (2*ACCEL) * (v*v - V_MIN*V_MIN);
    if(abs(pos - target) <= stoppingDist){
      v = max(v - ACCEL * stepDelay/1000000., V_MIN);
      rampUp = false;
    }
  }

  ///
  if(pos == target){
    v = 0;
  }
}
