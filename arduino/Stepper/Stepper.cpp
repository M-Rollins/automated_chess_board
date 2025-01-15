/*


*/

#include "Arduino.h"
#include "Stepper.h"

 Stepper::Stepper(int stepPin, int dirPin, boolean forward)
  {
    _stepPin = stepPin;
    _dirPin = dirPin;
    _forward = forward;
    _reverse = !forward;
    digitalWrite(stepPin, LOW);
    digitalWrite(dirPin, forward);
  }

void Stepper::pulse()
  {
    digitalWrite(_stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(_stepPin, LOW);
    delayMicroseconds(1000);
  }

  void Stepper::turn(int steps)
  {
    if(steps > 0)
    {
      digitalWrite(dirPin, forward);
    }else
    {
      digitalWrite(dirPin, reverse);
    }
    steps = abs(steps);
    for(int i = 0; i < steps; i++)
    {
      pulse();
    }
  }
