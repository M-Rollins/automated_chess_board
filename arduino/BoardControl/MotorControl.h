/*
 * MotorControl.h - library for control of stepper-driven motion axes
 */

#ifndef MotorControl_h
#define MotorControl_h

#include "Arduino.h"
// Class for controlling a stepper motor with A4988 stepper driver
class StepperMotor
{
  private: 
    static const int PULSE_WIDTH_MICROSECONDS;  //duration of pulse to trigger step pin
    int stepPin;
    int dirPin;
    boolean forward;  //signal to direction pin to move in the positive direction
  
  public:
    StepperMotor(int s, int d, boolean f);
    StepperMotor();
    void step(boolean d);
};


/*
 * controls a stepper-driven linear axis with a limit switch to reach a desired position
 */
class Axis
{
  private:
    StepperMotor motor; //motor which drives the axis
    int pos;  //current position (steps)
    int target; //target position
    int limitPos; //position of limit switch (assumed to be on upper limit of motion)
    int safetyLimit;  //value greater than limitPos that will trigger an error if the limit switch fails to trigger (broken switch, stalled motor, etc.)
    int limSwitch;  //pin associated with limit switch (should be pulled LOW when switch is triggered)
    boolean rampUp; //flag for if the motor is accelerating
    float v;  //speed (steps/s)
    unsigned long stepDelay;  //period between steps(us)
    unsigned long lastTime;

  public:
    boolean isHomed;  //flag for if the axis has a valid home position
    static const float V_MIN; //starting/stopping speed (steps/s)
    static const float V_MAX; //maximum speed
    static const float HOME_SPEED; //speed for homing the axes
    static const  float ACCEL;  //acceleration (steps/s^2)
    
    Axis(StepperMotor m, int lp, int switchPin);
    Axis();

    //runs to the limits switch, then returns true. Returns false if there is an error that prevents homing
    boolean homeAxis();
  
    //set position to move to
    void setTarget(int t);
  
    /* take steps as appropriate to rach target position
     * this function should be called as often as possible fro smooth motion
     */
    void updateAxis();

    boolean atTarget();
};

#endif
