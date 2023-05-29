#include "FastAccelStepper.h"

#if defined(__AVR_ATmega328P__)
  // Stepper Wiring
  #define dirPinStepper 8   //This can be any output capable port pin.
  #define stepPinStepper 9  // step pin must be pin 9, 10 or 11
  #define debugPinStepper  10

#elif defined(ARDUINO_ARCH_ESP32)
  // Stepper Wiring
  //#define dirPinStepper    8
  //#define stepPinStepper   9

  #define dirPinStepper    5
  #define stepPinStepper   17
  #define debugPinStepper  16

#endif

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

float steps_per_rev = 800;
float rpm = 2000;
float maxStepperSpeed = (rpm/60*steps_per_rev);   //needs to be in us per step || 1 sec = 1000000 us
//float maxStepperAccel = 1e2;
float maxStepperAccel = 1e8;



void setup()
{
  //Serial.begin(250000);
  Serial.begin(115200);

  delay(1000);


  //FastAccelStepper setup
  engine.init();
  //DRIVER_MCPWM_PCNT 
  //DRIVER_RMT
  stepper = engine.stepperConnectToPin(stepPinStepper, DRIVER_RMT);
  if (stepper) {

    Serial.println("Setup stepper!");
    
    //Stepper Parameters
    stepper->setDirectionPin(dirPinStepper, false);
    stepper->setAutoEnable(true);

    stepper->setSpeedInHz(maxStepperSpeed);   // steps/s
    stepper->setAcceleration(maxStepperAccel);  // 100 steps/s²


  }




  stepper->forceStopAndNewPosition(0);
  stepper->moveTo(0);


  pinMode(debugPinStepper, OUTPUT);    // sets the digital pin 13 as output
  
}

long currentTime = 0;
long elapsedTime = 0;
long previousTime = 0;

long Position_Next = 0;

//long targetCycleTime = 130;
long targetCycleTime = 1000;
//long targetCycleTime = 3000;

void loop()
{ 

  // obtain time
  currentTime = micros();
  elapsedTime = currentTime - previousTime;
  if (elapsedTime<1){elapsedTime=1;}

  // the average cycle time on my device is approx. 130 us --> mimic that
  long waitTime = targetCycleTime - elapsedTime;
  if (waitTime > 0){delayMicroseconds(waitTime); }

  //delay(500);

  //delay(1);
  currentTime = micros();
  elapsedTime = currentTime - previousTime;

  previousTime = currentTime;

  //Serial.println(elapsedTime);

  //Serial.println(stepper->rampState());
  

  

  // compute target position
  Position_Next += 500; // increment position
  Position_Next %= 10000; // reset positiony

  if (Position_Next == 0)
  {
    digitalWrite(debugPinStepper, HIGH);
  }
  else
  {
    digitalWrite(debugPinStepper, LOW);
  }

  //Serial.println(Position_Next);

  // add target position
  stepper->moveTo(Position_Next, false);

}