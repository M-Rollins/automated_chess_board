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
    int stepPin;  //pin sending step commands
    int dirPin; //pin which controls teh direction
    int ms1;  //microstepping pins
    int ms2;
    boolean forward;  //signal to direction pin to move in the positive direction
  
  public:
    StepperMotor(int s, int d, int msPin1, int msPin2, boolean f);
    StepperMotor();
    boolean setMicrostepping(int steps); //1 for full stepping, 2/4/8/ for half/quarter/eigth stepping. returns trus if successful
    void step(boolean d); //move the motor one step (positive direction if d is true)
};


/*
 * controls a stepper-driven linear axis with a limit switch to reach a desired position
 */
class Axis
{
  private:
    StepperMotor motor; //motor which drives the axis
    static const int GO_TO_TARGET;  //travel to the target position as quickly as posible and stop there
    static const int RAMP_CYCLE;  //follow a prespecified acceleration profile
    static const int SPLINE;  //follow a prespecified cubix motion profile
    int mode; //defines which of the above modes to follow
    //motion cycle parameters
    float t1; //ramp
    float t2; //ramp
    float tf; //ramp and spline
    float x0; //ramp and spline
    float v0;  //ramp and spline
    float a0; //ramp and spline
    float jerk; //spline
    
//    float cycleParams[5];  //for ramp cycle: [v0, a, t1, t2, t3]; for spline: [a, b, c, x0, tf]
    
    float pos;  //current position (full steps)
    float target; //target position
    int limitPos; //position of limit switch (assumed to be on upper limit of motion)
    int safetyLimit;  //value greater than limitPos that will trigger an error if the limit switch fails to trigger (broken switch, stalled motor, etc.)
    int limSwitch;  //pin associated with limit switch (should be pulled LOW when switch is triggered)
    boolean rampUp; //flag for if the motor is accelerating
    float v;  //speed (steps/s)
    float speedFactor; //scales acceleration and top speed
    unsigned long stepDelay;  //period between steps(us)
    unsigned long lastTime; //in position target mode: last time the motor stepped; in ramp and spline modes: start time of the motion profile
    int microstepping;  //number of microsteps each full step is broken into (1, 2, 4 or 8)

    //each motion mode has a separate position update function
    void updateGoToTarget();
    void updateRampCycle();
    void updateSpline();

  public:
    boolean isHomed;  //flag for if the axis has a valid home position
    static const float V_MIN; //starting/stopping speed (steps/s)
    static const float V_MAX; //maximum speed
    static const float HOME_SPEED; //speed for homing the axes
    static const float ACCEL;  //acceleration (steps/s^2)
    
    Axis(StepperMotor m, int lp, int switchPin);
    Axis();

    void setMicrostepping(int steps); //sets the microstepping resolution for the assigned motor and updates the motion commands accordingly
    boolean homeAxis(); //runs to the limits switch, then returns true. Returns false if there is an error that prevents homing
    void setSpeedFactor(float s);
    void setTarget(int t);  //set position to move to
    /*start a motion cycle: starting at speed v0, accelerate at rate a until time t1; constant velocity until t2; accelerate at -a until t3*/
    void followRampCycle(float v0, float a, float t1, float t2, float t3);
    /*follow a cubic curve (x(t) = at^3 + bt^2 + ct + x(0) until time tf*/
    void followSpline(float a, float b, float c, float tf);
  
    /* take steps as appropriate to reach target position
     * this function should be called as often as possible for smooth motion
     */
    void updateAxis();

    int getPos();
    boolean atTarget();
};

#endif
