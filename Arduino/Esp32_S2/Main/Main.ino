#define STEPPER_DELAY 0
#define ENDSTOP_MOVEMENT 5

#include <Joystick_ESP32S2.h>


Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   0, 0,                 // Button Count, Hat Switch Count
                   false, false, false,  // X and Y, but no Z Axis
                   false, false, false,  // No Rx, Ry, or Rz
                   false, false,         // No rudder or throttle
                   false, true, false);  // No accelerator, brake, or steering




// Arudino Sample Code to use ADS1256 library
// http://www.ti.com/lit/ds/symlink/ads1256.pdf

// Reads channel 0 continuously




#define STEPPER_MIN_OFFSET 2 * 500
#define STEPPER_MAX_OFFSET 2 * 500





float springStiffnesss = 1;
float springStiffnesssInv = 1;
float Force_Min = 0.1;    //Min Force in lb to activate Movement
float Force_Max = 7.;     //Max Force in lb = Max Travel Position
long stepperPosPrevious = 0;
long stepperPosCurrent = 0;
//long  Position_Deadzone = 1600. / 10.;  //Number of steps required before the pedal will move. Added in to prevent oscillation caused by varaying load cell readings
long  Position_Deadzone = 1;  //Number of steps required before the pedal will move. Added in to prevent oscillation caused by varaying load cell readings


#define minPin 11//4
#define maxPin 10//5


long stepperPosMin = 0;
long stepperPosMax = 0;
long stepperPosMin_global = 0;
long stepperPosMax_global = 0;
bool minEndstopNotTriggered = false;
bool maxEndstopNotTriggered = false;


// Configuration of Kalman filter
// assume constant rate of change 
// observed states:
// x = [force, d force / dt]
// state transition matrix
// x_k+1 = [1, delta_t; 0, 1] * x_k
#include <Kalman.h>
using namespace BLA;

// Dimensions of the matrices
#define KF_CONST_VEL

#ifdef KF_CONST_VEL
  #define Nstate 2 // length of the state vector
  #define Nobs 1   // length of the measurement vector
#else
  #define Nstate 3 // length of the state vector
  #define Nobs 1   // length of the measurement vector
#endif



// measurement std (to be characterized from your sensors)
#define n1 0.002 // noise on the 1st measurement component // standard deviation of measurement noise. Rule of thumb: assume Gaussian distribution. 99.9% is 3*sigma interval --> (max-min) / 3 = stddev
#define n2 0.1 // noise on the 2nd measurement component 

#define forceAccelerationError 100.0f

KALMAN<Nstate,Nobs> K; // your Kalman filter
BLA::Matrix<Nobs, 1> obs; // observation vector



#define LOADCELL_STD_MIN 0.001f
#define LOADCELL_VARIANCE_MIN LOADCELL_STD_MIN*LOADCELL_STD_MIN

float loadcellOffset = 0.0f;     //offset value
float varEstimate = 0.0f; // estimated loadcell variance
float stdEstimate = 0.0f;





//Stepper Motor Information

#include "FastAccelStepper.h"

// Stepper Wiring
#define dirPinStepper    8//17//8//12//3 
#define stepPinStepper   9//16//9//13//2  // step pin must be pin 9

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;



float steps_per_rev = 800;
float rpm = 1000;//2500;//4000;
float maxStepperSpeed = (rpm/60*steps_per_rev);   //needs to be in us per step || 1 sec = 1000000 us
float maxStepperAccel = 1e6;//5 * 1e6;//maxStepperSpeed * 100;

float startPosRel = 0.1;
float endPosRel = 0.7;

// Written by Axel Sepulveda, May 2020
#define ADC
#include <SPI.h>

/*#define FSPI_MISO   37
#define FSPI_MOSI   35
#define FSPI_SCLK   36
#define FSPI_SS     34 */

#include <ADS1256.h>


double conversion = 4000.;

float clockMHZ = 7.68; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference

// Construct and init ADS1256 object
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v

double sensor1;



double normalizePos(long pos, long Position_Min, long Position_Max)
{
  return ((double)(pos - Position_Min)) / ((double)(Position_Max - Position_Min));
}

double unnormalizePos(double relPos, long Position_Min, long Position_Max)
{
  double absPos = relPos * (double)(Position_Max - Position_Min) + (double)Position_Min;

  if (absPos > Position_Max)  {       //If current force is over the max force it will just read the max force
    absPos = Position_Max;
  }
  if (absPos < Position_Min)  {       //If current force is below the min force it will just read 0
    absPos = Position_Min;
  }

  return absPos;
}
  



void setup()
{
  Serial.begin(250000);
  //Serial.begin(9600);

  delay(1000);



  // define endstop switch
  //pinMode(minPin, INPUT_PULLUP);
  //pinMode(maxPin, INPUT_PULLUP);
  pinMode(minPin, INPUT);
  pinMode(maxPin, INPUT);



  
  

  Serial.println("Starting ADC");

  //delay(1000);
  
  // init ADC
  adc.initSpi(clockMHZ);

  delay(1000);

  Serial.println("ADS: send SDATAC command");
  adc.sendCommand(ADS1256_CMD_SDATAC);
  //adc.sendCommand(SDATAC);

  
#ifdef ADC

  

  // start the ADS1256 with data rate of 15 SPS and gain x1
  adc.begin(ADS1256_DRATE_15000SPS,ADS1256_GAIN_64,false); 
    
  // other data rates: 
  // ADS1256_DRATE_30000SPS
  // ADS1256_DRATE_15000SPS
  // ADS1256_DRATE_7500SPS
  // ADS1256_DRATE_3750SPS
  // ADS1256_DRATE_2000SPS
  // ADS1256_DRATE_1000SPS
  // ADS1256_DRATE_500SPS
  // ADS1256_DRATE_100SPS
  // ADS1256_DRATE_60SPS
  // ADS1256_DRATE_50SPS
  // ADS1256_DRATE_30SPS
  // ADS1256_DRATE_25SPS
  // ADS1256_DRATE_15SPS
  // ADS1256_DRATE_10SPS
  // ADS1256_DRATE_5SPS
  // ADS1256_DRATE_2_5SPS
  // 
  // NOTE : Data Rate vary depending on crystal frequency. Data rates listed below assumes the crystal frequency is 7.68Mhz
  //        for other frequency consult the datasheet.
  //Posible Gains 
  //ADS1256_GAIN_1 
  //ADS1256_GAIN_2 
  //ADS1256_GAIN_4 
  //ADS1256_GAIN_8 
  //ADS1256_GAIN_16 
  //ADS1256_GAIN_32 
  //ADS1256_GAIN_64 
  
  Serial.println("ADC Started");
  
  // Set MUX Register to AINO so it start doing the ADC conversion
  //Serial.println("Channel set to Single end ch0");
  //adc.setChannel(0);

//delay(1000);
  adc.waitDRDY(); // wait for DRDY to go low before changing multiplexer register
  //delay(1000);
  adc.setChannel(0,1);   // Set the MUX for differential between ch2 and 3 

  adc.setConversionFactor(conversion);
#endif






  Serial.println("ADC: Identify offset");
  // Due to construction and gravity, the loadcell measures an initial voltage difference.
  // To compensate this difference, the difference is estimated by moving average filter.
  float ival = 0;
  long samples = 1000;
  
  for (long i = 0; i < samples; i++){
    //adc.setChannel(0,1);   // Set the MUX for differential between ch2 and 3 
    //adc.waitDRDY(); // wait for DRDY to go low before next register read
    sensor1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and 
    ival = sensor1 / samples;
    Serial.println(sensor1,10);
    loadcellOffset += ival;
  }

  Serial.print("Offset ");
  Serial.println(loadcellOffset,10);
  //delay(1000);

  // automatically identify sensor noise for KF parameterization
  float varNormalizer = 1. / (samples - 1);
  varEstimate = 0.0f;
  for (long i = 0; i < samples; i++){
    //adc.setChannel(0,1);   // Set the MUX for differential between ch2 and 3 
    adc.waitDRDY(); // wait for DRDY to go low before next register read
    sensor1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and 
    ival = (sensor1 - loadcellOffset);
    ival *= ival;
    varEstimate += ival * varNormalizer;
    Serial.println(sensor1,10);
  }
  //varEstimate *= conversion * conversion; // respect linear scaling y = mu * x --> var(y) = mu^2 * var(x)

  // make sure estimate is nonzero
  if (varEstimate < LOADCELL_VARIANCE_MIN){varEstimate = LOADCELL_VARIANCE_MIN; }
  stdEstimate = sqrt(varEstimate);


  Serial.print("stdEstimate:");
  Serial.println(stdEstimate, 6);








      //FastAccelStepper setup
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  if (stepper) {

    Serial.println("Setup stepper!");
    stepper->setDirectionPin(dirPinStepper, false);
    //stepper->setDirectionPin(dirPinStepper);
    //stepper->setEnablePin(enablePinStepper);
    stepper->setAutoEnable(true);
    
  //Stepper Parameters
    stepper->setSpeedInHz(maxStepperSpeed);   // steps/s
    //stepper->setAcceleration(maxStepperAccel);  // 100 steps/s²

    //stepper->setSpeedInHz(20000);   // steps/s
    //stepper->setAcceleration(10000000);  // 100 steps/s²
    //stepper->setAcceleration(1e7);  // 100 steps/s²
    stepper->setAcceleration(1000000000);  // 100 steps/s²

    Serial.print("1:");
    Serial.print(10000000);
    Serial.print(",2:");
    Serial.print(1e7);
    Serial.print(",3:");
    Serial.print((int32_t)1e7);
    Serial.println("");



    //stepper->runForward();



#ifdef CONFIG_IDF_TARGET_ESP32S2
Serial.println("Got it!");
//Serial.println( engine._isValidStepPin(stepPinStepper) );
engine.setDebugLed(LED_BUILTIN);

Serial.println( getCpuFrequencyMhz() );

#endif
    delay(5000);



  }





  long set = 0;
  float slowMoveSpeed = maxStepperSpeed * 0.05;

  // Find min stepper position
  //Setting Max Limit Switch Position
  minEndstopNotTriggered = digitalRead(minPin);
  Serial.print(minEndstopNotTriggered);
  while(minEndstopNotTriggered == true){

    /*Serial.print("Pos:");
    Serial.print(set);
    Serial.print(", Min endstop state:");
    Serial.println(minEndstopNotTriggered);*/

    stepper->moveTo(set, true);
    delayMicroseconds(STEPPER_DELAY);


    minEndstopNotTriggered = digitalRead(minPin);
    set = set - ENDSTOP_MOVEMENT;
  }  
  stepper->forceStopAndNewPosition(0);
  stepper->moveTo(0);
  stepperPosMin_global = (long)stepper->getCurrentPosition();
  stepperPosMin = (long)stepper->getCurrentPosition() + STEPPER_MIN_OFFSET;


  Serial.println("The limit switch: Min On");
  Serial.print("Min Position is "); 
  Serial.println( stepperPosMin );


  // Find max stepper position
  set = 0;
  //Setting Max Limit Switch Position
  maxEndstopNotTriggered = digitalRead(maxPin);
  Serial.print(maxEndstopNotTriggered);
  while(maxEndstopNotTriggered == true){

    /*Serial.print("Pos:");
    Serial.print(set);
    Serial.print(", Max endstop state:");
    Serial.println(maxEndstopNotTriggered);*/

    stepper->moveTo(set, true);
    delayMicroseconds(STEPPER_DELAY);


    maxEndstopNotTriggered = digitalRead(maxPin);
    set = set + ENDSTOP_MOVEMENT;
  }  
  //s1.stop();
  stepperPosMax_global = (long)stepper->getCurrentPosition();
  stepperPosMax = (long)stepper->getCurrentPosition() - STEPPER_MAX_OFFSET;

  Serial.println("The limit switch: Max On");
  Serial.print("Max Position is "); 
  Serial.println( stepperPosMax );

  






  // correct start and end position as requested from the user
  float stepperRange = (stepperPosMax - stepperPosMin);
  stepperPosMin = stepperPosMin + stepperRange * startPosRel;
  stepperPosMax = stepperPosMin + stepperRange * endPosRel;

  // move to initial position
  stepper->moveTo(stepperPosMin, true);
  



  // compute pedal stiffness parameters
  springStiffnesss = (Force_Max-Force_Min) / (float)(stepperPosMax-stepperPosMin);
  springStiffnesssInv = 1.0 / springStiffnesss;

  // obtain current stepper position
  stepperPosPrevious = stepper->getCurrentPosition();









  // Kalman filter setup
  #ifdef KF_CONST_VEL
    // example of evolution matrix. Size is <Nstate,Nstate>
    K.F = {1.0, 0.0,
          0.0, 1.0};
          
    // example of measurement matrix. Size is <Nobs,Nstate>
    K.H = {1.0, 0.0};

    // example of model covariance matrix. Size is <Nstate,Nstate>
    K.Q = {1.0f,   0.0,
            0.0,  1.0};

  #else
    // example of evolution matrix. Size is <Nstate,Nstate>
    K.F = {1.0, 0.0, 0.0,
          0.0, 1.0, 0.0,
          0.0, 0.0, 1.0};
          
    // example of measurement matrix. Size is <Nobs,Nstate>
    K.H = {1.0, 0.0, 0.0};

    // example of model covariance matrix. Size is <Nstate,Nstate>
    K.Q = {1.0f,   0.0, 0.0,
            0.0,  1.0, 0.0,
            0.0,  0.0, 1.0};
  #endif
  

  // example of measurement covariance matrix. Size is <Nobs,Nobs>
  //K.R = {n1*n1};
  K.R = {varEstimate};




//Throttle Control Setup
  //Joystick.setAcceleratorRange(stepperPosMin, stepperPosMax);
  Joystick.setBrakeRange(0, 10000);
  delay(100);
  Joystick.begin();


  Serial.println("Setup end!");


}

long currentTime = 0;
long elapsedTime = 0;
long previousTime = 0;

double Force_Current_KF = 0.;


float averageCycleTime = 0.0f;

uint64_t maxCycles = 100;

uint64_t cycleIdx = 0;

int32_t joystickNormalizedToInt32 = 0;

float delta_t = 0.;
float delta_t_pow2 = 0.;
float delta_t_pow3 = 0.;
float delta_t_pow4 = 0.;

long Position_Next = 0;


void loop()
{ 

  // obtain time
  currentTime = micros();
  elapsedTime = currentTime - previousTime;
  if (elapsedTime<1){elapsedTime=1;}
  previousTime = currentTime;

//#define PRINT_CYCLETIME
#ifdef PRINT_CYCLETIME
  averageCycleTime += elapsedTime;
  cycleIdx++;
  if (maxCycles< cycleIdx)
  {
    cycleIdx = 0;

    averageCycleTime /= (float)maxCycles; 

    Serial.println(averageCycleTime);

    averageCycleTime = 0;
  }
#endif




#define RECALIBRATE_POSITION
#ifdef RECALIBRATE_POSITION
  // in case the stepper loses its position and therefore an endstop is triggered reset position
  minEndstopNotTriggered = digitalRead(minPin);
  maxEndstopNotTriggered = digitalRead(maxPin);

  if (minEndstopNotTriggered == false)
  {
    stepper->forceStopAndNewPosition(stepperPosMin_global);
  }
  if (maxEndstopNotTriggered == false)
  {
    stepper->forceStopAndNewPosition(stepperPosMax_global);
  }

#endif


#define READ_ADC 
#ifdef READ_ADC

  // read ADC value
  adc.waitDRDY(); // wait for DRDY to go low before next register read
  sensor1 = adc.readCurrentChannel(); // read as voltage according to gain and vref
  sensor1 -= loadcellOffset;

  // get current stepper position
  //stepperPosCurrent = stepper->getCurrentPosition();


  // get endstop psotiion
  minEndstopNotTriggered = digitalRead(minPin);
  maxEndstopNotTriggered = digitalRead(maxPin);
#endif


#define APPLY_KF
#ifdef APPLY_KF

  // Kalman filter  
  // update state transition and system covariance matrices
  delta_t = (float)elapsedTime / 1000000.0f; // convert to seconds

  delta_t_pow2 = delta_t * delta_t;
  delta_t_pow3 = delta_t_pow2 * delta_t;
  delta_t_pow4 = delta_t_pow2 * delta_t_pow2;

#ifdef KF_CONST_VEL

  K.F = {1.0,  delta_t, 
         0.0,  1.0};

  double K_Q_11 = forceAccelerationError * 0.5f * delta_t_pow3;
  K.Q = {forceAccelerationError * 0.25f * delta_t_pow4,   K_Q_11,
        K_Q_11, forceAccelerationError * delta_t_pow2};
        
#else

  K.F = {1.0,  delta_t, 0.5 * delta_t_pow2,
         0.0,  1.0, delta_t, 
         0.0, 0.0, 1.0};

  K.Q = {forceAccelerationError * 0.25f * delta_t_pow4,   forceAccelerationError * 0.5f * delta_t_pow3, forceAccelerationError * 0.5f * delta_t_pow2,
        forceAccelerationError * 0.5f * delta_t_pow3, forceAccelerationError * delta_t_pow2, forceAccelerationError * delta_t,
        forceAccelerationError * 0.5f * delta_t_pow2, forceAccelerationError * delta_t, forceAccelerationError};
#endif

  // APPLY KALMAN FILTER
  obs(0) = sensor1;
  K.update(obs);
  Force_Current_KF = K.x(0,0);




  // compute target position
  //Position_Next = springStiffnesssInv * (sensor1-Force_Min) + stepperPosMin ;        //Calculates new position using linear function
  Position_Next = springStiffnesssInv * (Force_Current_KF-Force_Min) + stepperPosMin ;        //Calculates new position using linear function

  if (Position_Next<stepperPosMin)
  {
    Position_Next = stepperPosMin;
  }

  if (Position_Next>stepperPosMax)
  {
    Position_Next = stepperPosMax;
  }

#endif







//#define PRINT_DEBUG
#ifdef PRINT_DEBUG
  Serial.print("elapsedTime:");
  Serial.print(elapsedTime);
  Serial.print(",instantaneousForceMeasured:");
  Serial.print(sensor1,6);
  Serial.print(",Kalman_x:");
  Serial.print(Force_Current_KF, 6);
  Serial.print(",Position_Next:");
  Serial.print(Position_Next, 6);

  //Serial.print(",Position_Next_PID:");
  //Serial.print(Position_Next_PID, 6);

  Serial.println(" ");
#endif


// get current stepper position
  stepperPosCurrent = stepper->getCurrentPosition();

#define SET_STEPPER
#ifdef SET_STEPPER
  //Position_Next = Position_Next_PID;
  if(abs(Position_Next-stepperPosCurrent)>Position_Deadzone){       //Checks to see if the new position move is greater than the deadzone limit set to prevent oscillation
    stepper->moveTo(Position_Next, false);

    delayMicroseconds(STEPPER_DELAY);
  } 
#endif


#define JOYSTICK_OUTPUT
#ifdef JOYSTICK_OUTPUT
  joystickNormalizedToInt32 =  ( Force_Current_KF - Force_Min) / (Force_Max-Force_Min)  * 10000.;
  Joystick.setBrake(joystickNormalizedToInt32);
#endif
  



  
  //stepper->moveTo(-4000, true);
  //stepper->moveTo(4000, true);
}