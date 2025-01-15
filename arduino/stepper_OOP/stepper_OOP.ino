int stepPin = 12;
int dirPin = 13;

class Stepper
{
  static const int pulseWidthMicroseconds = 1000;
  
  int stepPin;
  int dirPin;
  boolean forward;
  boolean reverse;
  
  void pulse()
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(pulseWidthMicroseconds);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(pulseWidthMicroseconds);
  }
  
  public:
  Stepper(int s, int d, boolean f)
  {
    stepPin = s;
    dirPin = d;
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    
    forward = f;
    reverse = !forward;
    digitalWrite(stepPin, LOW);
    digitalWrite(dirPin, forward);
  }
  
  void turn(int steps)
  {
    digitalWrite(dirPin, (steps > 0) == forward);

    steps = abs(steps);
    for(int i = 0; i < steps; i++)
    {
      pulse();
    }
  }
};


Stepper s1 = Stepper(12, 13, true);
void setup() {
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("forward");
  s1.turn(600);
  delay(1000);
  
  Serial.println("reverse");
  s1.turn(-600);
  delay(1000);
  
}
