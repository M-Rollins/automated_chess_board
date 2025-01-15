int stepPin = 12;
int dirPin = 13;

class StepperMotor
{
  int stepPin;
  int dirPin;
  boolean forward;
  boolean reverse;
  
  public:
  StepperMotor(int s, int d, boolean f)
  {
    stepPin = s;
    dirPin = d;
    forward = f;
    reverse = !forward;
    digitalWrite(stepPin, LOW);
    digitalWrite(dirPin, forward);
  }
  pulse()
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
  turn(int steps)
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
};

StepperMotor s1 = StepperMotor(12, 13, true);
void setup() {
  // put your setup code here, to run once:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("forward");
//  digitalWrite(dirPin, HIGH);
//  for (int i = 0; i < 200; i++)
//  {
//    s1.pulse();
//  }
  s1.turn(200);
  delay(1000);
  Serial.println("reverse");
  s1.turn(-200);
  delay(1000);
  
}

//void pulse()
//{
//  digitalWrite(stepPin, HIGH);
//  delayMicroseconds(1000);
//  digitalWrite(stepPin, LOW);
//  delayMicroseconds(1000);
//}
