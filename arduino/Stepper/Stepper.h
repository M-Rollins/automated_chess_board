/*


*/

#ifndef Stepper_h
#define Stepper_h

#include "Arduino.h"

class Stepper
{
  public:
    Stepper(int stepPin, int dirPin, boolean forward);
    void turn(int steps);

  private:
    int stepPin;
    int dirPin;
    boolean forward;
    boolean reverse;
    void pulse();
  
};

#endif
