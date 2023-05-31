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
long set = 0;






/**********************************************************************************************/
/*                                                                                            */
/*                         joystick  definitions                                              */
/*                                                                                            */
/**********************************************************************************************/
#include <Joystick_ESP32S2.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   0, 0,                 // Button Count, Hat Switch Count
                   false, false, false,  // X and Y, but no Z Axis
                   false, false, false,  // No Rx, Ry, or Rz
                   false, false,         // No rudder or throttle
                   false, true, false);  // No accelerator, brake, or steering


#define JOYSTICK_MIN_VALUE 0
#define JOYSTICK_MAX_VALUE 10000




/**********************************************************************************************/
/*                                                                                            */
/*                         endstop definitions                                                */
/*                                                                                            */
/**********************************************************************************************/
#define minPin 11
#define maxPin 10
#define ENDSTOP_MOVEMENT 5 // movement per cycle to find endstop positions

long stepperPosMin = 0;
long stepperPosMax = 0;
long stepperPosMin_global = 0;
long stepperPosMax_global = 0;
bool minEndstopNotTriggered = false;
bool maxEndstopNotTriggered = false;
long stepperPosPrevious = 0;
long stepperPosCurrent = 0;


/**********************************************************************************************/
/*                                                                                            */
/*                         pedal mechanics definitions                                        */
/*                                                                                            */
/**********************************************************************************************/
float startPosRel = 0.1;
float endPosRel = 0.7;

float springStiffnesss = 1;
float springStiffnesssInv = 1;
float Force_Min = 0.1;    //Min Force in lb to activate Movement
float Force_Max = 7.;     //Max Force in lb = Max Travel Position
double conversion = 4000.;


/**********************************************************************************************/
/*                                                                                            */
/*                         Kalman filter definitions                                          */
/*                                                                                            */
/**********************************************************************************************/
#include <Kalman.h>
using namespace BLA;
// Configuration of Kalman filter
// assume constant rate of change 
// observed states:
// x = [force, d force / dt]
// state transition matrix
// x_k+1 = [1, delta_t; 0, 1] * x_k


// Dimensions of the matrices
#define KF_CONST_VEL
#define Nstate 2 // length of the state vector
#define Nobs 1   // length of the measurement vector
#define KF_MODEL_NOISE_FORCE_ACCELERATION (float)10.0f // adjust model noise here

KALMAN<Nstate,Nobs> K; // your Kalman filter
BLA::Matrix<Nobs, 1> obs; // observation vector



/**********************************************************************************************/
/*                                                                                            */
/*                         loadcell definitions                                               */
/*                                                                                            */
/**********************************************************************************************/
#define LOADCELL_STD_MIN 0.001f
#define LOADCELL_VARIANCE_MIN LOADCELL_STD_MIN*LOADCELL_STD_MIN

float loadcellOffset = 0.0f;     //offset value
float varEstimate = 0.0f; // estimated loadcell variance
float stdEstimate = 0.0f;



/**********************************************************************************************/
/*                                                                                            */
/*                         stepper motor definitions                                          */
/*                                                                                            */
/**********************************************************************************************/

#include "FastAccelStepper.h"

// Stepper Wiring
#define dirPinStepper    8//17//8//12//3 
#define stepPinStepper   9//16//9//13//2  // step pin must be pin 9

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;


#define STEPS_PER_MOTOR_REVOLUTION (float)800.0f
#define MAXIMUM_STEPPER_RPM (float)4000.0f
#define MAXIMUM_STEPPER_SPEED (MAXIMUM_STEPPER_RPM/60*STEPS_PER_MOTOR_REVOLUTION)   //needs to be in us per step || 1 sec = 1000000 us
#define SLOW_STEPPER_SPEED (float)(MAXIMUM_STEPPER_SPEED * 0.05f)
#define MAXIMUM_STEPPER_ACCELERATION (float)1e9




/**********************************************************************************************/
/*                                                                                            */
/*                         ADC definitions                                                    */
/*                                                                                            */
/**********************************************************************************************/
#include <SPI.h>
#include <ADS1256.h>
#define NUMBER_OF_SAMPLES_FOR_LOADCELL_OFFFSET_ESTIMATION 1000

float clockMHZ = 7.68; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference

// Construct and init ADS1256 object
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v
double loadcellReading;




  


/**********************************************************************************************/
/*                                                                                            */
/*                         setup function                                                     */
/*                                                                                            */
/**********************************************************************************************/
void setup()
{
  Serial.begin(115200);

  delay(1000);


  // define endstop switch
  pinMode(minPin, INPUT);
  pinMode(maxPin, INPUT);


  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);


  Serial.println("Starting ADC");  
  adc.initSpi(clockMHZ);
  delay(1000);
  Serial.println("ADS: send SDATAC command");
  //adc.sendCommand(ADS1256_CMD_SDATAC);

  
  // start the ADS1256 with data rate of 15kSPS SPS and gain x64
  adc.begin(ADS1256_DRATE_15000SPS,ADS1256_GAIN_64,false); 
  Serial.println("ADC Started");

  adc.waitDRDY(); // wait for DRDY to go low before changing multiplexer register
  adc.setChannel(0,1);   // Set the MUX for differential between ch2 and 3 
  adc.setConversionFactor(conversion);



  Serial.println("ADC: Identify loadcell offset");
  // Due to construction and gravity, the loadcell measures an initial voltage difference.
  // To compensate this difference, the difference is estimated by moving average filter.
  float ival = 0;
  loadcellOffset = 0.0f;
  for (long i = 0; i < NUMBER_OF_SAMPLES_FOR_LOADCELL_OFFFSET_ESTIMATION; i++){
    loadcellReading = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and 
    ival = loadcellReading / (float)NUMBER_OF_SAMPLES_FOR_LOADCELL_OFFFSET_ESTIMATION;
    Serial.println(loadcellReading,10);
    loadcellOffset += ival;
  }

  Serial.print("Offset ");
  Serial.println(loadcellOffset,10);



  // automatically identify sensor noise for KF parameterization
  Serial.println("ADC: Identify loadcell variance");
  float varNormalizer = 1. / (float)(NUMBER_OF_SAMPLES_FOR_LOADCELL_OFFFSET_ESTIMATION - 1);
  varEstimate = 0.0f;
  for (long i = 0; i < NUMBER_OF_SAMPLES_FOR_LOADCELL_OFFFSET_ESTIMATION; i++){
    adc.waitDRDY(); // wait for DRDY to go low before next register read
    loadcellReading = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and 
    ival = (loadcellReading - loadcellOffset);
    ival *= ival;
    varEstimate += ival * varNormalizer;
    Serial.println(loadcellReading,10);
  }

  // make sure estimate is nonzero
  if (varEstimate < LOADCELL_VARIANCE_MIN){varEstimate = LOADCELL_VARIANCE_MIN; }
  stdEstimate = sqrt(varEstimate);

  Serial.print("stdEstimate:");
  Serial.println(stdEstimate, 6);



  //FastAccelStepper setup
  if (stepper) {

    Serial.println("Setup stepper!");
    stepper->setDirectionPin(dirPinStepper, false);
    stepper->setAutoEnable(true);
    
    //Stepper Parameters
    stepper->setSpeedInHz(MAXIMUM_STEPPER_SPEED);   // steps/s
    stepper->setAcceleration(MAXIMUM_STEPPER_ACCELERATION);  // 100 steps/s²

    delay(5000);
  }





  

  // Find min stepper position
  minEndstopNotTriggered = digitalRead(minPin);
  Serial.print(minEndstopNotTriggered);
  while(minEndstopNotTriggered == true){
    stepper->moveTo(set, true);
    minEndstopNotTriggered = digitalRead(minPin);
    set = set - ENDSTOP_MOVEMENT;
  }  
  stepper->forceStopAndNewPosition(0);
  stepper->moveTo(0);
  stepperPosMin_global = (long)stepper->getCurrentPosition();
  stepperPosMin = (long)stepper->getCurrentPosition();

  Serial.println("The limit switch: Min On");
  Serial.print("Min Position is "); 
  Serial.println( stepperPosMin );


  // Find max stepper position
  set = 0;
  maxEndstopNotTriggered = digitalRead(maxPin);
  Serial.print(maxEndstopNotTriggered);
  while(maxEndstopNotTriggered == true){
    stepper->moveTo(set, true);
    maxEndstopNotTriggered = digitalRead(maxPin);
    set = set + ENDSTOP_MOVEMENT;
  }  
  stepperPosMax_global = (long)stepper->getCurrentPosition();
  stepperPosMax = (long)stepper->getCurrentPosition();

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
    // example of evolution matrix. Size is <Nstate,Nstate>
    K.F = {1.0, 0.0,
          0.0, 1.0};
          
    // example of measurement matrix. Size is <Nobs,Nstate>
    K.H = {1.0, 0.0};

    // example of model covariance matrix. Size is <Nstate,Nstate>
    K.Q = {1.0f,   0.0,
            0.0,  1.0};

  // example of measurement covariance matrix. Size is <Nobs,Nobs>
  K.R = {varEstimate};




  //Throttle Control Setup
  Joystick.setBrakeRange(JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
  delay(100);
  Joystick.begin();


  Serial.println("Setup end!");

  previousTime = micros();


}







/**********************************************************************************************/
/*                                                                                            */
/*                         Main function                                                      */
/*                                                                                            */
/**********************************************************************************************/
void loop()
{ 

  #define RECALIBRATE_POSITION_FROM_SERIAL
  #ifdef RECALIBRATE_POSITION_FROM_SERIAL
    byte n = Serial.available();
    if(n !=0 )
    {
      int menuChoice = Serial.parseInt();
      
      switch (menuChoice) {
        // resset minimum position
        case 1:

          //Serial.println("Reset position!");
          set = stepperPosMin_global;
          while(minEndstopNotTriggered == true){
            stepper->moveTo(set, true);
            minEndstopNotTriggered = digitalRead(minPin);
            set = set - ENDSTOP_MOVEMENT;
          }  
          stepper->forceStopAndNewPosition(stepperPosMin_global);
          //stepper->moveTo(0);
          
          break;

        // toggle ABS
        case 2:
          Serial.print("Second case:");
          break;

        default:
          Serial.print("Default case:");
      }
    }
  #endif



    // obtain time
    currentTime = micros();
    elapsedTime = currentTime - previousTime;
    if (elapsedTime<1){elapsedTime=1;}
    previousTime = currentTime;

  // average execution time averaged over multiple cycles 
  #define PRINT_CYCLETIME
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
      stepper->moveTo(stepperPosMin, true);
    }
    if (maxEndstopNotTriggered == false)
    {
      stepper->forceStopAndNewPosition(stepperPosMax_global);
      stepper->moveTo(stepperPosMax, true);
    }

  #endif

    // read ADC value
    adc.waitDRDY(); // wait for DRDY to go low before next register read
    loadcellReading = adc.readCurrentChannel(); // read as voltage according to gain and vref
    loadcellReading -= loadcellOffset;


    // Kalman filter  
    // update state transition and system covariance matrices
    delta_t = (float)elapsedTime / 1000000.0f; // convert to seconds
    delta_t_pow2 = delta_t * delta_t;
    delta_t_pow3 = delta_t_pow2 * delta_t;
    delta_t_pow4 = delta_t_pow2 * delta_t_pow2;

    K.F = {1.0,  delta_t, 
          0.0,  1.0};

    double K_Q_11 = KF_MODEL_NOISE_FORCE_ACCELERATION * 0.5f * delta_t_pow3;
    K.Q = {KF_MODEL_NOISE_FORCE_ACCELERATION * 0.25f * delta_t_pow4,   K_Q_11,
          K_Q_11, KF_MODEL_NOISE_FORCE_ACCELERATION * delta_t_pow2};
          

    // APPLY KALMAN FILTER
    obs(0) = loadcellReading;
    K.update(obs);
    Force_Current_KF = K.x(0,0);


    // compute target position
    Position_Next = springStiffnesssInv * (Force_Current_KF-Force_Min) + stepperPosMin ;        //Calculates new position using linear function
    Position_Next = (int32_t)constrain(Position_Next, stepperPosMin, stepperPosMax);
    

  #define SET_STEPPER
  #ifdef SET_STEPPER
    // get current stepper position
    //stepperPosCurrent = stepper->getCurrentPosition();
    stepper->moveTo(Position_Next, false);
  #endif


  #define JOYSTICK_OUTPUT
  #ifdef JOYSTICK_OUTPUT
    joystickNormalizedToInt32 =  ( Force_Current_KF - Force_Min) / (Force_Max-Force_Min)  * JOYSTICK_MAX_VALUE;
    joystickNormalizedToInt32 = (int32_t)constrain(joystickNormalizedToInt32, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
    Joystick.setBrake(joystickNormalizedToInt32);
  #endif


  //#define PRINT_DEBUG
  #ifdef PRINT_DEBUG
    Serial.print("elapsedTime:");
    Serial.print(elapsedTime);
    Serial.print(",instantaneousForceMeasured:");
    Serial.print(loadcellReading,6);
    Serial.print(",Kalman_x:");
    Serial.print(Force_Current_KF, 6);
    Serial.print(",Position_Next:");
    Serial.print(Position_Next, 6);
    Serial.println(" ");

    delay(100);
  #endif

}