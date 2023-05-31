#include "FastAccelStepper.h"

long chirpTimeInitial = 0;

#if defined(__AVR_ATmega328P__)
  // Stepper Wiring
  #define dirPinStepper 8   //This can be any output capable port pin.
  #define stepPinStepper 9  // step pin must be pin 9, 10 or 11
  #define debugPinStepper  10

#elif defined(ARDUINO_ARCH_ESP32)
  // Stepper Wiring

  #define dirPinStepper    8
  #define stepPinStepper   9

  //#define dirPinStepper    5
  //#define stepPinStepper   17
  #define debugPinStepper  16

#endif

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;

float steps_per_rev = 800;
float rpm = 4000;
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

  #if defined(ARDUINO_ARCH_ESP32)
    //stepper = engine.stepperConnectToPin(stepPinStepper, DRIVER_RMT);
    stepper = engine.stepperConnectToPin(stepPinStepper);
  #else
    stepper = engine.stepperConnectToPin(stepPinStepper);
  #endif

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

  chirpTimeInitial = micros();
  
}

long currentTime = 0;
long elapsedTime = 0;
long previousTime = 0;

long Position_Next = 0;

long targetCycleTime = 130;


float f_0 = 0.5; // Hz, initial freuqncy of oscillation
float f_ramp = 0.1; // Hz / s  frequency ramp of oscillation
float f = 0;
float positionOscillation;
float oscilattionAmplitude = 500;
void loop()
{ 

  float chirpTime = ( micros() - chirpTimeInitial ) * 1e-6;

  // generate chirp signal
  f = f_0 + f_ramp * chirpTime;
  positionOscillation = cos( 2 * PI * f * chirpTime);
  positionOscillation *= oscilattionAmplitude;

  // if frequency increases the maximum frequency, reset and repeat
  if (f > 50)
  {
    chirpTimeInitial = micros();
  }

  // obtain time
  currentTime = micros();
  elapsedTime = currentTime - previousTime;
  if (elapsedTime<1){elapsedTime=1;}

  // the average cycle time on my device is approx. 130 us --> mimic that
  long waitTime = targetCycleTime - elapsedTime;
  if (waitTime > 0){delayMicroseconds(waitTime); }

  previousTime = currentTime;

  // add target position
  stepper->moveTo(positionOscillation, false);
}