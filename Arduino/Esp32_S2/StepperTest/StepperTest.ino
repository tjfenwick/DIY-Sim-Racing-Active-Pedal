#include "FastAccelStepper.h"

// Stepper Wiring
#define dirPinStepper    8
#define stepPinStepper   9

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

float steps_per_rev = 800;
float rpm = 1000;
float maxStepperSpeed = (rpm/60*steps_per_rev);   //needs to be in us per step || 1 sec = 1000000 us
float maxStepperAccel = 1e2;



void setup()
{
  Serial.begin(250000);

  delay(1000);


  //FastAccelStepper setup
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
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
  
}

long currentTime = 0;
long elapsedTime = 0;
long previousTime = 0;

long Position_Next = 0;

long targetCycleTime = 130;

void loop()
{ 

  // obtain time
  currentTime = micros();
  elapsedTime = currentTime - previousTime;
  if (elapsedTime<1){elapsedTime=1;}

  // the average cycle time on my device is approx. 130 us --> mimic that
  long waitTime = targetCycleTime - elapsedTime;
  if (waitTime > 0){delayMicroseconds(waitTime); }

  previousTime = currentTime;

  // compute target position
  Position_Next += 500; // increment position
  Position_Next %= 10000; // reset position

  Serial.println(Position_Next);

  // add target position
  stepper->moveTo(Position_Next, false);

}