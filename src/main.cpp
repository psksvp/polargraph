#include <Arduino.h>
#include <AccelStepper.h>           

#define sign(n)  (n < 0 ? -1 : 1)

#define FULLSTEP 4
#define HALFSTEP 8

#define motorPin1  8                // IN1 pin on the ULN2003A driver
#define motorPin2  9                // IN2 pin on the ULN2003A driver
#define motorPin3  10               // IN3 pin on the ULN2003A driver
#define motorPin4  11               // IN4 pin on the ULN2003A driver
#define motorPin5  4                // Blue   - 28BYJ48 pin 1
#define motorPin6  5                // Pink   - 28BYJ48 pin 2
#define motorPin7  6                // Yellow - 28BYJ48 pin 3
#define motorPin8  7                // Orange - 28BYJ48 pin 4


// must alway be halfstep for my stepper and its controller.
AccelStepper rightMotor(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
AccelStepper leftMotor(HALFSTEP, motorPin5, motorPin7, motorPin6, motorPin8);

// config
// const float ropeThickness = 0.25;  //compensation when rope roll up 
const float cmPerTurn = 11;
const int stepsPerRevolution = 4096;
const float motorSpeed = 40;
const float motorAccel = 4;
const float motorMaxSpeed = 100;

// states
bool running = false;
float currentLeftLength = 77;
float currentRightLength = 77;

//function decl
void idle();
void blink();
void moveMotor(float lengthCM, AccelStepper& s);
void turnMotor(float revolution, AccelStepper& s);
void rotateMotor(float angle, AccelStepper& s);
void runSerialRead(void(*read)(const String&));
void commandReceived(const String& s);
void setWiresLength(float newLeftLength, 
                    float newRightLength, 
                    bool pen);



// functions code

void reportReady()
{
  Serial.print("OK ");
  Serial.print(currentLeftLength);
  Serial.print(" ");
  Serial.println(currentRightLength);
}


//func
void setup() 
{
  Serial.begin(9600); 
  reportReady();
  pinMode(LED_BUILTIN, OUTPUT);

  rightMotor.setMaxSpeed(motorMaxSpeed);      
  rightMotor.setAcceleration(motorAccel);   
  rightMotor.setSpeed(motorSpeed);   

  leftMotor.setMaxSpeed(motorMaxSpeed);      
  leftMotor.setAcceleration(motorAccel);   
  leftMotor.setSpeed(motorSpeed);     
}

void loop() 
{
  rightMotor.run();                    // keep moving the motor
  leftMotor.run();
  
  if(!running)
    idle();
  else 
  {
    if(0 == rightMotor.distanceToGo() && 0 == leftMotor.distanceToGo())
    {
      reportReady();
      running = false;
    }
  }
}

void commandReceived(const String& s)
{
  static float b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0 ,0};

  int i = 0;
  int f = 0;
  int idx = s.indexOf(',', f);
  while(i < 10 && idx >= 0)
  {
    b[i] = s.substring(f, idx).toFloat();
    f = idx + 1;
    i = i + 1;
    idx = s.indexOf(',', f);
    if(i < 10 && idx < 0 && f < s.length()) // capture the last one 
    {
      b[i] = s.substring(f).toFloat();
    }
  }
  running = true;
  setWiresLength(b[0], b[1], true);
}

void setWiresLength(float newLeftLength, 
                    float newRightLength, 
                    bool pen)
{
  float leftDelta =  newLeftLength - currentLeftLength;
  float rightDelta =  newRightLength - currentRightLength;
  Serial.println(leftDelta);
  Serial.println(rightDelta);
  moveMotor(leftDelta, leftMotor);
  moveMotor(rightDelta, rightMotor);
  currentLeftLength = currentLeftLength + leftDelta;
  currentRightLength = currentRightLength + rightDelta;
}

void moveMotor(float lengthCM, AccelStepper& s)
{
  turnMotor(lengthCM / cmPerTurn, s);
}

void turnMotor(float revolution, AccelStepper& s)
{
  long d = (long)(revolution * stepsPerRevolution);
  long p = s.currentPosition() + d;
  s.moveTo(p);
}

void rotateMotor(float angle, AccelStepper& s)
{
  long d = (long)(angle * stepsPerRevolution / 360.0);
  s.moveTo(s.currentPosition() + d);
}

void runSerialRead(void(*read)(const String&))
{
  static String command = "";
  if (Serial.available()) 
  {
    char inChar = (char)Serial.read();
    command += inChar;
    if (inChar == '\n') 
    {
      read(command);
      command = "";
    }
  }
}

void idle()
{
  runSerialRead(commandReceived);
  blink();
}

void blink()
{
  static int state = HIGH;
  digitalWrite(LED_BUILTIN, state);   
  state = HIGH == state ? LOW : HIGH;
  delay(50);
}
