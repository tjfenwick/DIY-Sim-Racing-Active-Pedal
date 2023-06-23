long currentTime = 0;
long elapsedTime = 0;
long previousTime = 0;
double Force_Current_KF = 0.;
double Force_Current_KF_dt = 0.;
float averageCycleTime = 0.0f;
uint64_t maxCycles = 1000;
uint64_t cycleIdx = 0;
int32_t joystickNormalizedToInt32 = 0;
float delta_t = 0.;
float delta_t_pow2 = 0.;
float delta_t_pow3 = 0.;
float delta_t_pow4 = 0.;
long Position_Next = 0;
long set = 0;
bool checkPosition = 1;

bool absActive = 0;
float absTime = 0;
float stepperAbsOffset = 0;
float absDeltaTimeSinceLastTrigger = 0;


bool resetPedalPosition = false;
bool configUpdateAvailable = false;





#include "DiyActivePedal_types.h"
DAP_config_st dap_config_st;
DAP_config_st dap_config_st_local;
DAP_calculationVariables_st dap_calculationVariables_st;


int32_t pcnt = 0;

#define RAD_2_DEG 180.0f / PI





//USBCDC USBSerial;

#define MIN_STEPS 5

//#define SUPPORT_ESP32_PULSE_COUNTER


//#define PRINT_CYCLETIME

#define ABS_SCALING 50



/**********************************************************************************************/
/*                                                                                            */
/*                         iterpolation  definitions                                          */
/*                                                                                            */
/**********************************************************************************************/

#define INTERP_SPRING_STIFFNESS
#ifdef INTERP_SPRING_STIFFNESS
  #include "InterpolationLib.h"

  #define INTERPOLATION_NUMBER_OF_SOURCE_VALUES 6
  #define INTERPOLATION_NUMBER_OF_TARGET_VALUES 30

  double xValues[INTERPOLATION_NUMBER_OF_SOURCE_VALUES] = { 0, 20, 40, 60, 80, 100 };
  double yValues[INTERPOLATION_NUMBER_OF_SOURCE_VALUES] = { 0, 20, 40, 60, 80, 100 };

  float interpTargetValues[INTERPOLATION_NUMBER_OF_TARGET_VALUES];
  float interpSpringStiffness[INTERPOLATION_NUMBER_OF_TARGET_VALUES];
  float interpStepperPos[INTERPOLATION_NUMBER_OF_TARGET_VALUES];

  


  // stiffness = Force_Range / stepperPosRange
  // 
  void generateStiffnessCurve()
  {

    xValues[0] = dap_calculationVariables_st.stepperPosMin + 0.00 * dap_calculationVariables_st.stepperPosRange;
    xValues[1] = dap_calculationVariables_st.stepperPosMin + 0.20 * dap_calculationVariables_st.stepperPosRange;
    xValues[2] = dap_calculationVariables_st.stepperPosMin + 0.40 * dap_calculationVariables_st.stepperPosRange;
    xValues[3] = dap_calculationVariables_st.stepperPosMin + 0.60 * dap_calculationVariables_st.stepperPosRange;
    xValues[4] = dap_calculationVariables_st.stepperPosMin + 0.80 * dap_calculationVariables_st.stepperPosRange;
    xValues[5] = dap_calculationVariables_st.stepperPosMin + 1.00 * dap_calculationVariables_st.stepperPosRange;


    
    yValues[0] = dap_calculationVariables_st.Force_Min + ((float)dap_config_st.relativeForce_p000) / 100.0f * dap_calculationVariables_st.Force_Range;
    yValues[1] = dap_calculationVariables_st.Force_Min + ((float)dap_config_st.relativeForce_p020) / 100.0f * dap_calculationVariables_st.Force_Range;
    yValues[2] = dap_calculationVariables_st.Force_Min + ((float)dap_config_st.relativeForce_p040) / 100.0f* dap_calculationVariables_st.Force_Range;
    yValues[3] = dap_calculationVariables_st.Force_Min + ((float)dap_config_st.relativeForce_p060) / 100.0f * dap_calculationVariables_st.Force_Range;
    yValues[4] = dap_calculationVariables_st.Force_Min + ((float)dap_config_st.relativeForce_p080) / 100.0f * dap_calculationVariables_st.Force_Range;
    yValues[5] = dap_calculationVariables_st.Force_Min + ((float)dap_config_st.relativeForce_p100) / 100.0f * dap_calculationVariables_st.Force_Range;

    for (uint8_t tmp = 0; tmp < 6; tmp++)
    {
      
      Serial.print(xValues[tmp]);
      Serial.print("  ");
      Serial.print(yValues[tmp]);
      Serial.println(" ");  
    }

    Serial.println(" ");
    Serial.println(" ");
    Serial.println("Interp values: ");
    for (uint interpStep = 0; interpStep < INTERPOLATION_NUMBER_OF_TARGET_VALUES; interpStep++)
    {
      double xValueSample = ((double)interpStep) / ((double)INTERPOLATION_NUMBER_OF_TARGET_VALUES);
      xValueSample = dap_calculationVariables_st.stepperPosMin + xValueSample * dap_calculationVariables_st.stepperPosRange;

      //interpTargetValues[interpStep] = Interpolation::Linear(xValues, yValues, INTERPOLATION_NUMBER_OF_SOURCE_VALUES, xValueSample, false);
      //interpTargetValues[interpStep] = Interpolation::SmoothStep(xValues, yValues, INTERPOLATION_NUMBER_OF_SOURCE_VALUES, xValueSample);
      interpTargetValues[interpStep] = Interpolation::CatmullSpline(xValues, yValues, INTERPOLATION_NUMBER_OF_SOURCE_VALUES, xValueSample);
      //interpTargetValues[interpStep] = Interpolation::ConstrainedSpline(xValues, yValues, INTERPOLATION_NUMBER_OF_SOURCE_VALUES, xValueSample);
      
      interpStepperPos[interpStep] = xValueSample;
      
      Serial.print(xValueSample);
      Serial.print("   ");
      Serial.println(interpTargetValues[interpStep]);
    }


    Serial.println(" ");
    Serial.println(" ");
    Serial.println("Stiffness: ");
    Serial.println(dap_calculationVariables_st.springStiffnesss);


    for (uint interpStep = 0; interpStep < (INTERPOLATION_NUMBER_OF_TARGET_VALUES-1); interpStep++)
    {
      interpSpringStiffness[interpStep] = abs( interpTargetValues[interpStep+1] - interpTargetValues[interpStep]);
      interpSpringStiffness[interpStep] /=  dap_calculationVariables_st.stepperPosRange / ( INTERPOLATION_NUMBER_OF_TARGET_VALUES-1);

      Serial.print(interpStep);
      Serial.print("   ");
      Serial.println(interpSpringStiffness[interpStep]);

    }
    interpSpringStiffness[INTERPOLATION_NUMBER_OF_TARGET_VALUES-1] = interpSpringStiffness[INTERPOLATION_NUMBER_OF_TARGET_VALUES-2];
  }

#endif



/**********************************************************************************************/
/*                                                                                            */
/*                         multitasking  definitions                                          */
/*                                                                                            */
/**********************************************************************************************/
#include "soc/rtc_wdt.h"

//rtc_wdt_protect_off();    // Turns off the automatic wdt service
//rtc_wdt_enable();         // Turn it on manually
//rtc_wdt_set_time(RTC_WDT_STAGE0, 20000);  // Define how long you desire to let dog wait.



TaskHandle_t Task1;
TaskHandle_t Task2;
//SemaphoreHandle_t batton;
//SemaphoreHandle_t semaphore_updateJoystick;

static SemaphoreHandle_t semaphore_updateConfig=NULL;
static SemaphoreHandle_t semaphore_updateJoystick=NULL;


/**********************************************************************************************/
/*                                                                                            */
/*                         target-specific  definitions                                       */
/*                                                                                            */
/**********************************************************************************************/

// Wiring connections
#if CONFIG_IDF_TARGET_ESP32S2
  #define minPin 11
  #define maxPin 10
  #define dirPinStepper    8//17//8//12//3 
  #define stepPinStepper   9//16//9//13//2  // step pin must be pin 9
  #define USB_JOYSTICK
#elif CONFIG_IDF_TARGET_ESP32
  #define minPin 34
  #define maxPin 35
  #define dirPinStepper    0//8
  #define stepPinStepper   4//9
  #define BLUETOOTH_GAMEPAD
#endif


/**********************************************************************************************/
/*                                                                                            */
/*                         controller  definitions                                            */
/*                                                                                            */
/**********************************************************************************************/

#define JOYSTICK_MIN_VALUE 0
#define JOYSTICK_MAX_VALUE 10000

#if defined USB_JOYSTICK
  #include <Joystick_ESP32S2.h>
  
  Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   0, 0,                 // Button Count, Hat Switch Count
                   false, false, false,  // X and Y, but no Z Axis
                   false, false, false,  // No Rx, Ry, or Rz
                   false, false,         // No rudder or throttle
                   false, true, false);  // No accelerator, brake, or steering
  
  void SetupController() {
    Joystick.setBrakeRange(JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
    delay(100);
    Joystick.begin();
  }
  bool IsControllerReady() { return true; }
  void SetControllerOutputValue(int32_t value) {
    Joystick.setBrake(value);
  }
  
#elif defined BLUETOOTH_GAMEPAD
  #include <BleGamepad.h>

  BleGamepad bleGamepad("DiyActiveBrake", "DiyActiveBrake", 100);
  
  void SetupController() {
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_MULTI_AXIS); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setAxesMin(JOYSTICK_MIN_VALUE); // 0 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
    bleGamepadConfig.setAxesMax(JOYSTICK_MAX_VALUE); // 32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal 
    //bleGamepadConfig.setWhichSpecialButtons(false, false, false, false, false, false, false, false);
    //bleGamepadConfig.setWhichAxes(false, false, false, false, false, false, false, false);
    bleGamepadConfig.setWhichSimulationControls(false, false, false, true, false); // only brake active 
    bleGamepadConfig.setButtonCount(0);
    bleGamepadConfig.setHatSwitchCount(0);
    bleGamepadConfig.setAutoReport(false);
    bleGamepad.begin(&bleGamepadConfig);
  }

  bool IsControllerReady() { return bleGamepad.isConnected(); }

  void SetControllerOutputValue(int32_t value) {
    //bleGamepad.setBrake(value);
    bleGamepad.setAxes(value, 0, 0, 0, 0, 0, 0, 0);
    bleGamepad.sendReport();
    //Serial.println(value);
  }
  
#endif






/**********************************************************************************************/
/*                                                                                            */
/*                         pedal mechanics definitions                                        */
/*                                                                                            */
/**********************************************************************************************/
float startPosRel = 0.35;
float endPosRel = 0.8;
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
#define KF_MODEL_NOISE_FORCE_ACCELERATION (float)( 2.0f * 9.0f / 0.05f/ 0.05f )// adjust model noise here s = 0.5 * a * delta_t^2 --> a = 2 * s / delta_t^2

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

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;


#define TRAVEL_PER_ROTATION_IN_MM (float)5.0f
//#define STEPS_PER_MOTOR_REVOLUTION (float)300.0f
#define STEPS_PER_MOTOR_REVOLUTION (float)1600.0f
#define MAXIMUM_STEPPER_RPM (float)7000.0f
#define MAXIMUM_STEPPER_SPEED (MAXIMUM_STEPPER_RPM / 60.0*STEPS_PER_MOTOR_REVOLUTION)   //needs to be in us per step || 1 sec = 1000000 us
#define SLOW_STEPPER_SPEED (float)(MAXIMUM_STEPPER_SPEED * 0.15f)
#define MAXIMUM_STEPPER_ACCELERATION (float)1e9




/**********************************************************************************************/
/*                                                                                            */
/*                         endstop definitions                                                */
/*                                                                                            */
/**********************************************************************************************/
#define ENDSTOP_MOVEMENT STEPS_PER_MOTOR_REVOLUTION / 100.0f // movement per cycle to find endstop positions

bool minEndstopNotTriggered = false;
bool maxEndstopNotTriggered = false;
long stepperPosPrevious = 0;
long stepperPosCurrent = 0;


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
/*                         helper function                                                    */
/*                                                                                            */
/**********************************************************************************************/



// initialize configuration struct at startup
void initConfig()
{

  dap_config_st.payloadType = 100;
  dap_config_st.version = 0;
  dap_config_st.pedalStartPosition = 35;
  dap_config_st.pedalEndPosition = 80;

  dap_config_st.maxForce = 90;
  dap_config_st.preloadForce = 1;

  dap_config_st.relativeForce_p000 = 0;
  dap_config_st.relativeForce_p020 = 20;
  dap_config_st.relativeForce_p040 = 40;
  dap_config_st.relativeForce_p060 = 60;
  dap_config_st.relativeForce_p080 = 80;
  dap_config_st.relativeForce_p100 = 100;

  dap_config_st.dampingPress = 0;
  dap_config_st.dampingPull = 0;

  dap_config_st.absFrequency = 2 * PI * 10;
  dap_config_st.absAmplitude = 100.0f;



  dap_config_st.lengthPedal_AC = 150;
  dap_config_st.horPos_AB = 215;
  dap_config_st.verPos_AB = 80;
  dap_config_st.lengthPedal_CB = 200;
}

// update the local variables used for computation from the config struct
void updateComputationalVariablesFromConfig()
{

  dap_calculationVariables_st.startPosRel = ((float)dap_config_st.pedalStartPosition) / 100.0f;
  dap_calculationVariables_st.endPosRel = ((float)dap_config_st.pedalEndPosition) / 100.0f;

  dap_calculationVariables_st.absFrequency = 2 * PI * ((float)dap_config_st.absFrequency);
  dap_calculationVariables_st.absAmplitude = ((float)dap_config_st.absAmplitude)/ TRAVEL_PER_ROTATION_IN_MM * STEPS_PER_MOTOR_REVOLUTION / ABS_SCALING; // in mm

  dap_calculationVariables_st.dampingPress = ((float)dap_config_st.dampingPress) / 400.0f;
  

  // update force variables
  dap_calculationVariables_st.Force_Min = ((float)dap_config_st.preloadForce) / 10.0f;
  dap_calculationVariables_st.Force_Max = ((float)dap_config_st.maxForce) / 10.0f;
}

// compute pedal incline angle
float computePedalInclineAngle(float sledPosInCm)
{

  // see https://de.wikipedia.org/wiki/Kosinussatz
  // A: is lower pedal pivot
  // C: is upper pedal pivot
  // B: is rear pedal pivot
  float a = ((float)dap_config_st.lengthPedal_CB) / 10.0f;
  float b = ((float)dap_config_st.lengthPedal_AC) / 10.0f;
  float c_ver = ((float)dap_config_st.verPos_AB) / 10.0f;
  float c_hor = ((float)dap_config_st.horPos_AB) / 10.0f;
  c_hor += sledPosInCm / 10.0f;
  float c = sqrtf(c_ver * c_ver + c_hor * c_hor);

  /*Serial.print("a: ");
  Serial.print(a);

  Serial.print(", b: ");
  Serial.print(b);

  Serial.print(", c: ");
  Serial.print(c);

  Serial.print(", sledPosInCm: ");
  Serial.print(sledPosInCm);*/

  float nom = b*b + c*c - a*a;
  float den = 2 * b * c;

  float alpha = 0;
   
  if (abs(den) > 0.01)
  {
    alpha = acos( nom / den );
  }

  
  /*Serial.print(", alpha1: ");
  Serial.print(alpha * RAD_2_DEG);*/


  // add incline due to AB incline --> result is incline realtive to horizontal 
  if (abs(c_hor)>0.01)
  {
    alpha += atan(c_ver / c_hor);
  }

  /*Serial.print(", alpha2: ");
  Serial.print(alpha * RAD_2_DEG);
  Serial.println(" ");*/
  
  return alpha * RAD_2_DEG;
  
}


/**********************************************************************************************/
/*                                                                                            */
/*                         setup function                                                     */
/*                                                                                            */
/**********************************************************************************************/
void setup()
{
  //Serial.begin(115200);
  Serial.begin(921600);
  Serial.setTimeout(5);

  

  


  //batton = xSemaphoreCreateBinary();
  //semaphore_updateJoystick = xSemaphoreCreateBinary();
  semaphore_updateJoystick = xSemaphoreCreateMutex();
  semaphore_updateConfig = xSemaphoreCreateMutex();


  if(semaphore_updateJoystick==NULL)
  {
    Serial.println("Could not create semaphore");
    ESP.restart();
  }

  disableCore0WDT();


  // initialize configuration and update local variables
  initConfig();
  updateComputationalVariablesFromConfig();

  // init controller
  SetupController();


  //USBSerial.begin(921600);

  delay(1000);


  // define endstop switch
  pinMode(minPin, INPUT);
  pinMode(maxPin, INPUT);


  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  //stepper = engine.stepperConnectToPin(stepPinStepper, DRIVER_RMT);


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
    //Serial.println(loadcellReading,10);
    loadcellOffset += ival;
  }

  Serial.print("Offset ");
  Serial.println(loadcellOffset,10);



  // automatically identify sensor noise for KF parameterization
  #ifdef ESTIMATE_LOADCELL_VARIANCE
    Serial.println("ADC: Identify loadcell variance");
    float varNormalizer = 1. / (float)(NUMBER_OF_SAMPLES_FOR_LOADCELL_OFFFSET_ESTIMATION - 1);
    varEstimate = 0.0f;
    for (long i = 0; i < NUMBER_OF_SAMPLES_FOR_LOADCELL_OFFFSET_ESTIMATION; i++){
      adc.waitDRDY(); // wait for DRDY to go low before next register read
      loadcellReading = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and 
      ival = (loadcellReading - loadcellOffset);
      ival *= ival;
      varEstimate += ival * varNormalizer;
      //Serial.println(loadcellReading,10);
    }

    // make sure estimate is nonzero
    if (varEstimate < LOADCELL_VARIANCE_MIN){varEstimate = LOADCELL_VARIANCE_MIN; }
    varEstimate *= 9;
  #else
    varEstimate = 0.2f * 0.2f;
  #endif
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

#if defined(SUPPORT_ESP32_PULSE_COUNTER)
    stepper->attachToPulseCounter(1, 0, 0);
#endif

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
  dap_calculationVariables_st.stepperPosMinEndstop = (long)stepper->getCurrentPosition();

  Serial.println("The limit switch: Min On");
  Serial.print("Min Position is "); 
  Serial.println( dap_calculationVariables_st.stepperPosMinEndstop );


  // Find max stepper position
  set = 0;
  maxEndstopNotTriggered = digitalRead(maxPin);
  Serial.print(maxEndstopNotTriggered);
  while(maxEndstopNotTriggered == true){
    stepper->moveTo(set, true);
    maxEndstopNotTriggered = digitalRead(maxPin);
    set = set + ENDSTOP_MOVEMENT;
  } 
  Serial.print(maxEndstopNotTriggered);
  dap_calculationVariables_st.stepperPosMaxEndstop = (long)stepper->getCurrentPosition();
  
  Serial.println("The limit switch: Max On");
  Serial.print("Max Position is "); 
  Serial.println( dap_calculationVariables_st.stepperPosMaxEndstop );

  // compute pedal stiffness parameters
  update_pedal_stiffness(&dap_calculationVariables_st);
  #ifdef INTERP_SPRING_STIFFNESS
    generateStiffnessCurve();
  #endif

  // move to initial position
  stepper->moveTo(dap_calculationVariables_st.stepperPosMin, true);
#if defined(SUPPORT_ESP32_PULSE_COUNTER)
  stepper->clearPulseCounter();
#endif

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









/*#if defined(SUPPORT_ESP32_PULSE_COUNTER)
  stepper->attachToPulseCounter(1, 0, 0);
#endif*/


  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    pedalUpdateTask,   /* Task function. */
                    "pedalUpdateTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
    delay(500);

  xTaskCreatePinnedToCore(
                    serialCommunicationTask,   
                    "serialCommunicationTask", 
                    10000,     
                    NULL,      
                    1,         
                    &Task2,    
                    1);     
    delay(500);




  // initialize the interpolation curve
  //
  #ifdef INTERP_SPRING_STIFFNESS

    



    

  #endif



  // equalize pedal config for both tasks
  dap_config_st_local = dap_config_st;

  Serial.println("Setup end!");

  previousTime = micros();




}







/**********************************************************************************************/
/*                                                                                            */
/*                         Main function                                                      */
/*                                                                                            */
/**********************************************************************************************/
void loop() {
}


/**********************************************************************************************/
/*                                                                                            */
/*                         pedal update task                                                  */
/*                                                                                            */
/**********************************************************************************************/

long cycleIdx2 = 0;


  //void loop()
  void pedalUpdateTask( void * pvParameters )
  {

    for(;;){

      // obtain time
      currentTime = micros();
      elapsedTime = currentTime - previousTime;
      if (elapsedTime<1){elapsedTime=1;}
      previousTime = currentTime;

      // print the execution time averaged over multiple cycles 
      #ifdef PRINT_CYCLETIME
        averageCycleTime += elapsedTime;
        cycleIdx++;
        if (maxCycles< cycleIdx)
        {
          cycleIdx = 0;
          averageCycleTime /= (float)maxCycles; 
          Serial.print("PU cycle time: ");
          Serial.println(averageCycleTime);
          averageCycleTime = 0;
        }
      #endif


      // if a config update was received over serial, update the variables required for further computation
      if (configUpdateAvailable == true)
      {
        if(xSemaphoreTake(semaphore_updateConfig, 1)==pdTRUE)
        {
          Serial.println("Update pedal config!");
          configUpdateAvailable = false;
          dap_config_st = dap_config_st_local;
          updateComputationalVariablesFromConfig();
          update_pedal_stiffness(&dap_calculationVariables_st);
          #ifdef INTERP_SPRING_STIFFNESS
            generateStiffnessCurve();
          #endif
          xSemaphoreGive(semaphore_updateConfig);
        }
      }



      // if reset pedal position was requested, reset pedal now
      // This function is implemented, so that in case of lost steps, the user can request a reset of the pedal psotion
      if (resetPedalPosition)
      {
        set = 0;
        minEndstopNotTriggered = digitalRead(minPin);
        Serial.println(minEndstopNotTriggered);
        while(minEndstopNotTriggered == true){
          stepper->moveTo(set, true);
          minEndstopNotTriggered = digitalRead(minPin);
          set = set - ENDSTOP_MOVEMENT;
        }  
        stepper->forceStopAndNewPosition(dap_calculationVariables_st.stepperPosMinEndstop);
        resetPedalPosition = false;
      }


      //#define RECALIBRATE_POSITION
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
  

      // compute pedal oscillation, when ABS is active
    #define ABS_OSCILLATION
    #ifdef ABS_OSCILLATION  
      if (absActive)
      {
        absTime += elapsedTime * 1e-6; 
        absDeltaTimeSinceLastTrigger += elapsedTime * 1e-6; 
        stepperAbsOffset = dap_calculationVariables_st.absAmplitude * sin(dap_calculationVariables_st.absFrequency * absTime);
      }
      
      // reset ABS when trigger is not active anymore
      if (absDeltaTimeSinceLastTrigger > 0.1)
      {
        absTime = 0;
        absActive = false;
      }
    #endif


    // compute the pedal incline angle 
    //#define COMPUTE_PEDAL_INCLINE_ANGLE
    #ifdef COMPUTE_PEDAL_INCLINE_ANGLE
      float sledPosition = ((float)stepperPosCurrent) / STEPS_PER_MOTOR_REVOLUTION * TRAVEL_PER_ROTATION_IN_MM;
      float pedalInclineAngle = computePedalInclineAngle(sledPosition);
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
      Force_Current_KF_dt = K.x(0,1);

      

      // use interpolation to determine local linearized spring stiffness
      #ifndef INTERP_SPRING_STIFFNESS

        float spingStiffnessInv_lcl = dap_calculationVariables_st.springStiffnesssInv;
        // caclulate pedal position
        Position_Next = spingStiffnessInv_lcl * (Force_Current_KF - dap_calculationVariables_st.Force_Min) + dap_calculationVariables_st.stepperPosMin ;        //Calculates new position using linear function

      #else

        stepperPosCurrent = stepper->getCurrentPosition();
        float interpPosition = stepperPosCurrent - dap_calculationVariables_st.stepperPosMin;
        interpPosition /= (dap_calculationVariables_st.stepperPosMax - dap_calculationVariables_st.stepperPosMin);
        interpPosition *= (float)INTERPOLATION_NUMBER_OF_TARGET_VALUES;
        uint8_t  sriffnessArrayPos = (uint8_t)constrain(interpPosition, 0, INTERPOLATION_NUMBER_OF_TARGET_VALUES-1);



        
        float spingStiffnessInv_lcl = dap_calculationVariables_st.springStiffnesssInv;
        if (interpSpringStiffness[sriffnessArrayPos] > 0)
        {
          //spingStiffnessInv_lcl *= (1.0f / interpSpringStiffness[sriffnessArrayPos]);
          spingStiffnessInv_lcl = (1.0f / interpSpringStiffness[sriffnessArrayPos]);
        }
        

        // caclulate pedal position
        Position_Next = spingStiffnessInv_lcl * (Force_Current_KF - interpTargetValues[sriffnessArrayPos]) + interpStepperPos[sriffnessArrayPos] ;

      #endif

      
      

      // add dampening
      if (dap_calculationVariables_st.dampingPress  > 0.0001)
      {
        // dampening is proportional to velocity --> D-gain for stability
        Position_Next -= dap_calculationVariables_st.dampingPress * Force_Current_KF_dt * dap_calculationVariables_st.springStiffnesssInv;
      }
      


    #ifdef ABS_OSCILLATION
      Position_Next += stepperAbsOffset;
    #endif
      // clip target position to configured target interval
      Position_Next = (int32_t)constrain(Position_Next, dap_calculationVariables_st.stepperPosMin, dap_calculationVariables_st.stepperPosMax);


      // get current stepper position right before sheduling a new move
      //stepperPosCurrent = stepper->getCurrentPosition();
      stepperPosCurrent = stepper->getPositionAfterCommandsCompleted();
      long movement = abs( stepperPosCurrent - Position_Next);
      if (movement>MIN_STEPS  )
      {
        stepper->moveTo(Position_Next, false);
      }

      // compute controller output
      if(abs( dap_calculationVariables_st.Force_Range )>0.01)
      {     
        int32_t joystickNormalizedToInt32_local = ( Force_Current_KF - dap_calculationVariables_st.Force_Min) / dap_calculationVariables_st.Force_Range * JOYSTICK_MAX_VALUE;
        if(xSemaphoreTake(semaphore_updateJoystick, 1)==pdTRUE)
        {
          joystickNormalizedToInt32 = (int32_t)constrain(joystickNormalizedToInt32_local, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
          xSemaphoreGive(semaphore_updateJoystick);
        }
      }
    }
  }

  








/**********************************************************************************************/
/*                                                                                            */
/*                         communication task                                                 */
/*                                                                                            */
/**********************************************************************************************/

  unsigned long sc_currentTime = 0;
  unsigned long sc_previousTime = 0;
  unsigned long sc_elapsedTime = 0;
  unsigned long sc_cycleIdx = 0;
  float sc_averageCycleTime = 0;
  int32_t joystickNormalizedToInt32_local = 0;

  void serialCommunicationTask( void * pvParameters )
  {

    for(;;){

    // average cycle time averaged over multiple cycles 
    #ifdef PRINT_CYCLETIME

      // obtain time
      sc_currentTime = micros();
      sc_elapsedTime = sc_currentTime - sc_previousTime;
      if (sc_elapsedTime<1){sc_elapsedTime=1;}
      sc_previousTime = sc_currentTime;
      
      sc_averageCycleTime += sc_elapsedTime;
      sc_cycleIdx++;
      if (maxCycles < sc_cycleIdx)
      {
        sc_cycleIdx = 0;
        sc_averageCycleTime /= (float)maxCycles; 
        Serial.print("SC cycle time: ");
        Serial.println(sc_averageCycleTime);
        sc_averageCycleTime = 0;
      }
    #endif





      // read serial input 
      byte n = Serial.available();

      // likely config structure 
      if ( n == sizeof(DAP_config_st) )
      {
        
        if(xSemaphoreTake(semaphore_updateConfig, 1)==pdTRUE)
        {
          DAP_config_st * dap_config_st_local_ptr;
          dap_config_st_local_ptr = &dap_config_st_local;
          Serial.readBytes((char*)dap_config_st_local_ptr, sizeof(DAP_config_st));

          Serial.println("Config received!");

          // check if data is plausible
          bool structChecker = true;
          if ( dap_config_st_local.payloadType != dap_config_st.payloadType ){ structChecker = false;}
          if ( dap_config_st_local.version != dap_config_st.version ){ structChecker = false;}

          // if checks are successfull, overwrite global configuration struct
          if (structChecker == true)
          {
            configUpdateAvailable = true;          
          }
          xSemaphoreGive(semaphore_updateConfig);
        }
      }
      else
      {
        if (n!=0)
        {
          int menuChoice = Serial.parseInt();
          switch (menuChoice) {
            // resset minimum position
            case 1:
              Serial.println("Reset position!");
              resetPedalPosition = true;
              break;

            // toggle ABS
            case 2:
              //Serial.print("Second case:");
              absActive = true;
              absDeltaTimeSinceLastTrigger = 0;
              break;

            default:
              Serial.println("Default case:");
              break;
          }

        }
      }




      // transmit controller output
      if (IsControllerReady())
      {

        delay(1);
        if(xSemaphoreTake(semaphore_updateJoystick, 1)==pdTRUE)
        {
          joystickNormalizedToInt32_local = joystickNormalizedToInt32;
          xSemaphoreGive(semaphore_updateJoystick);
        }
        SetControllerOutputValue(joystickNormalizedToInt32_local);  
      }

    }
  }
