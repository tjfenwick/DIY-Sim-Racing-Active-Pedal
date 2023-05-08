// Arudino Sample Code to use ADS1256 library
// http://www.ti.com/lit/ds/symlink/ads1256.pdf

// Reads channel 0 continuously






//#define USE_PID_CONTROLLER
#ifdef USE_PID_CONTROLLER
#include <PID_v1.h>
// Add PID controller to compute the travel distance
//Specify the links and initial tuning parameters
//double Kp=0.5, Ki=0.01, Kd=0.3;
double Kp=1., Ki=0.00, Kd=0.0;
double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

double antiWindup = 0;
double pidLastError = 0.0f;
double pidLastInput = 0.0f;
double pidIntegrator = 0.0f;

#endif




float springStiffnesss = 1;
float springStiffnesssInv = 1;
float Force_Min = 0.1;    //Min Force in lb to activate Movement
float Force_Max = 3.;     //Max Force in lb = Max Travel Position
long stepperPosPrevious = 0;
long stepperPosCurrent = 0;
//long  Position_Deadzone = 1600. / 10.;  //Number of steps required before the pedal will move. Added in to prevent oscillation caused by varaying load cell readings
long  Position_Deadzone = 10;  //Number of steps required before the pedal will move. Added in to prevent oscillation caused by varaying load cell readings


#define minPin 34
#define maxPin 35


long stepperPosMin = 0;
long stepperPosMax = 0;
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

#define forceAccelerationError 0.5f

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
#define dirPinStepper    0 
#define stepPinStepper   4  // step pin must be pin 9

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;



float steps_per_rev = 1600;
float rpm = 4000;
float maxStepperSpeed = (rpm/60*steps_per_rev);   //needs to be in us per step || 1 sec = 1000000 us
float maxStepperAccel = maxStepperSpeed * 100;


// Written by Axel Sepulveda, May 2020
#define ADC
#include <SPI.h>
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
    stepper->setDirectionPin(dirPinStepper, false);
    //stepper->setEnablePin(enablePinStepper);
    stepper->setAutoEnable(true);
    
  //Stepper Parameters
    stepper->setSpeedInHz(maxStepperSpeed);   // steps/s
    stepper->setAcceleration(maxStepperAccel);  // 100 steps/s²

  }








  // Find max stepper position
  long set = 0;
  float slowMoveSpeed = maxStepperSpeed * 0.05;
  //Setting Max Limit Switch Position
  maxEndstopNotTriggered = digitalRead(maxPin);
  Serial.print(maxEndstopNotTriggered);
  while(maxEndstopNotTriggered == true){

    /*Serial.print("Pos:");
    Serial.print(set);
    Serial.print(", Max endstop state:");
    Serial.println(maxEndstopNotTriggered);*/

    stepper->moveTo(set, true);


    maxEndstopNotTriggered = digitalRead(maxPin);
    set = set + 20;
  }  
  //s1.stop();
  stepperPosMax = (long)stepper->getCurrentPosition() - 1000;

  Serial.println("The limit switch: Max On");
  Serial.print("Max Position is "); 
  Serial.println( stepperPosMax );




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


    minEndstopNotTriggered = digitalRead(minPin);
    set = set - 20;
  }  
  //s1.stop();
  stepperPosMin = (long)stepper->getCurrentPosition() + 1000;

  Serial.println("The limit switch: Min On");
  Serial.print("Min Position is "); 
  Serial.println( stepperPosMin );



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



#ifdef USE_PID_CONTROLLER
  //turn the PID on
  Input = 0;
  Setpoint = 0;
  myPID.SetSampleTime(1);
  myPID.SetMode(AUTOMATIC);
#endif



}

long currentTime = 0;
long elapsedTime = 0;
long previousTime = 0;

double Force_Current_KF = 0.;

void loop()
{ 

  // obtain time
  currentTime = micros();
  elapsedTime = currentTime - previousTime;
  if (elapsedTime<1){elapsedTime=1;}
  previousTime = currentTime;


  // read ADC value
  adc.waitDRDY(); // wait for DRDY to go low before next register read
  sensor1 = adc.readCurrentChannel(); // read as voltage according to gain and vref
  sensor1 -= loadcellOffset;

  // get current stepper position
  stepperPosCurrent = stepper->getCurrentPosition();


  // get endstop psotiion
  minEndstopNotTriggered = digitalRead(minPin);
  maxEndstopNotTriggered = digitalRead(maxPin);






  // Kalman filter  
  // update state transition and system covariance matrices
  float delta_t = (float)elapsedTime / 1000000.0f; // convert to seconds

  

  float delta_t_pow2 = delta_t * delta_t;
  float delta_t_pow3 = delta_t_pow2 * delta_t;
  float delta_t_pow4 = delta_t_pow3 * delta_t;

#ifdef KF_CONST_VEL

  K.F = {1.0,  delta_t, 
         0.0,  1.0};

  K.Q = {forceAccelerationError * 0.25f * delta_t_pow4,   forceAccelerationError * 0.5f * delta_t_pow3,
        forceAccelerationError * 0.5f * delta_t_pow3, forceAccelerationError * delta_t_pow2};
        
#else

  K.F = {1.0,  delta_t, 0.5 * delta_t * delta_t,
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
  long Position_Next = springStiffnesssInv * (Force_Current_KF-Force_Min) + stepperPosMin ;        //Calculates new position using linear function







#ifdef USE_PID_CONTROLLER
  // originally a P (k_p=1) controller is used to calculate the target position.
  // the target psoition only depends on the current difference between target position and current position.
  // By using the I and D components, the target position also depends on the accumulated difference and the change of the difference.
  // Proper parameter tuning might help to reduce/remove pedal oscilations.


  // Use PID controller to compute the next position
  // travelDistance is the residue
  Setpoint = normalizePos(Position_Next, stepperPosMin, stepperPosMax); // normalized target position
  Input = normalizePos(stepperPosCurrent, stepperPosMin, stepperPosMax); // normalized current position

  
  //myPID.Compute(); // PID computes normalized target position


  double error = Setpoint - Input;
  double error_d = error - pidLastError;
  double input_d = (Input - pidLastInput) / delta_t;
  pidLastError = error;
  pidLastInput = Input;

  // Integrator with anti windup

  // 
  


  pidIntegrator += error * delta_t;
  pidIntegrator += antiWindup * delta_t;

  // http://brettbeauregard.com/blog/2011/04/improving-the-beginner%E2%80%99s-pid-reset-windup/
  double outMax = 1.0f / Ki;
  double outMin = -1.0f / Ki;
  //if(pidIntegrator> outMax) pidIntegrator= outMax;
  //else if(pidIntegrator< outMin) pidIntegrator= outMin;


  //Output = Kp * error + Ki * pidIntegrator + Kd * error_d;
  Output = Kp * error + Ki * pidIntegrator - Kd * input_d;

  
  if (Output > 1)
  {
    antiWindup = 1. - Output;
    Output = 1.0;
  }

  if (Output < 0)
  {
    antiWindup = 0. - Output;
    Output = 0.0;
  }


  /*Serial.print("pidSetpoint:");
  Serial.print(Setpoint, 5);
  Serial.print(",");

  Serial.print("pidInput:");
  Serial.print(Input, 5);
  Serial.print(",");

  Serial.print("pidOutput:");
  Serial.print(Output, 5);
  Serial.print(",");

  Serial.print("targetPos:");
  Serial.print((long)unnormalizePos(Output));
  Serial.print(",");

  Serial.print("Position_Next:");
  Serial.print(Position_Next);
  Serial.print(",");*/

  // unnormalize PID output and set as new target position
  double Position_Next_PID = (long)unnormalizePos(Output, stepperPosMin, stepperPosMax);
  
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

  Serial.print(",Position_Next_PID:");
  Serial.print(Position_Next_PID, 6);

  Serial.println(" ");
#endif


  //Position_Next = Position_Next_PID;
  if(abs(Position_Next-stepperPosCurrent)>Position_Deadzone){       //Checks to see if the new position move is greater than the deadzone limit set to prevent oscillation
    stepper->moveTo(Position_Next, false);
  }


  /*Serial.print("Min:");
  Serial.print(minEndstopNotTriggered); // Print as decimal, 10 decimal places
  Serial.print(",Max:");
  Serial.print(maxEndstopNotTriggered); // Print as decimal, 10 decimal places
  Serial.println(" ");*/



  
  //stepper->moveTo(-4000, true);
  //stepper->moveTo(4000, true);
}
