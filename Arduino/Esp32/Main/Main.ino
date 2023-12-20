#define ESTIMATE_LOADCELL_VARIANCE
#define ISV_COMMUNICATION
//#define PRINT_SERVO_STATES

#define DEBUG_INFO_0_CYCLE_TIMER 1
#define DEBUG_INFO_0_STEPPER_POS 2
#define DEBUG_INFO_0_LOADCELL_READING 4
#define DEBUG_INFO_0_SERVO_READINGS 8
#define DEBUG_INFO_0_PRINT_ALL_SERVO_REGISTERS 16


bool resetServoEncoder = true;
bool isv57LifeSignal_b = false;
#ifdef ISV_COMMUNICATION
  #include "isv57communication.h"
  int32_t servo_offset_compensation_steps_i32 = 0; 
#endif





#include "Main.h"

//#define ALLOW_SYSTEM_IDENTIFICATION

/**********************************************************************************************/
/*                                                                                            */
/*                         function declarations                                              */
/*                                                                                            */
/**********************************************************************************************/
void updatePedalCalcParameters();


bool systemIdentificationMode_b = false;





bool splineDebug_b = false;



#include <EEPROM.h>



#include "ABSOscillation.h"
ABSOscillation absOscillation;
#define ABS_OSCILLATION



#include "DiyActivePedal_types.h"
DAP_config_st dap_config_st;
DAP_calculationVariables_st dap_calculationVariables_st;




#include "CycleTimer.h"





#include "RTDebugOutput.h"


/**********************************************************************************************/
/*                                                                                            */
/*                         iterpolation  definitions                                          */
/*                                                                                            */
/**********************************************************************************************/

#include "ForceCurve.h"
ForceCurve_Interpolated forceCurve;



/**********************************************************************************************/
/*                                                                                            */
/*                         multitasking  definitions                                          */
/*                                                                                            */
/**********************************************************************************************/
#include "soc/rtc_wdt.h"

//#define PRINT_USED_STACK_SIZE
// https://stackoverflow.com/questions/55998078/freertos-task-priority-and-stack-size
#define STACK_SIZE_FOR_TASK_1 0.2 * (configTOTAL_HEAP_SIZE / 4)
#define STACK_SIZE_FOR_TASK_2 0.2 * (configTOTAL_HEAP_SIZE / 4)


TaskHandle_t Task1;
TaskHandle_t Task2;
#ifdef ISV_COMMUNICATION
  isv57communication isv57;
  #define STACK_SIZE_FOR_TASK_3 0.2 * (configTOTAL_HEAP_SIZE / 4) 
  TaskHandle_t Task3;
#endif

static SemaphoreHandle_t semaphore_updateConfig=NULL;
  bool configUpdateAvailable = false;                              // semaphore protected data
  DAP_config_st dap_config_st_local;

static SemaphoreHandle_t semaphore_updateJoystick=NULL;
  int32_t joystickNormalizedToInt32 = 0;                           // semaphore protected data

static SemaphoreHandle_t semaphore_resetServoPos=NULL;
bool resetPedalPosition = false;


/**********************************************************************************************/
/*                                                                                            */
/*                         target-specific  definitions                                       */
/*                                                                                            */
/**********************************************************************************************/




/**********************************************************************************************/
/*                                                                                            */
/*                         controller  definitions                                            */
/*                                                                                            */
/**********************************************************************************************/

#include "Controller.h"




/**********************************************************************************************/
/*                                                                                            */
/*                         pedal mechanics definitions                                        */
/*                                                                                            */
/**********************************************************************************************/

#include "PedalGeometry.h"


/**********************************************************************************************/
/*                                                                                            */
/*                         Kalman filter definitions                                          */
/*                                                                                            */
/**********************************************************************************************/

#include "SignalFilter.h"
KalmanFilter* kalman = NULL;


/**********************************************************************************************/
/*                                                                                            */
/*                         FIR notch filter definitions                                       */
/*                                                                                            */
/**********************************************************************************************/

#include "SignalFilterFirNotch.h"
FirNotchFilter* firNotchFilter = NULL;



/**********************************************************************************************/
/*                                                                                            */
/*                         loadcell definitions                                               */
/*                                                                                            */
/**********************************************************************************************/

#include "LoadCell.h"
LoadCell_ADS1256* loadcell = NULL;



/**********************************************************************************************/
/*                                                                                            */
/*                         stepper motor definitions                                          */
/*                                                                                            */
/**********************************************************************************************/

#include "StepperWithLimits.h"
StepperWithLimits* stepper = NULL;
//static const int32_t MIN_STEPS = 5;

#include "StepperMovementStrategy.h"








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

  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");

  // init controller
  SetupController();
  delay(2000);


// check whether iSV57 communication can be established
// and in case, (a) send tuned servo parameters and (b) prepare the servo for signal read
#ifdef ISV_COMMUNICATION
  // check whether iSV57 is connected
  isv57LifeSignal_b = isv57.checkCommunication();

  Serial.print("iSV57 communication state:  ");
  Serial.println(isv57LifeSignal_b);

  if (isv57LifeSignal_b)
  {
    isv57.setupServoStateReading();
  	isv57.sendTunedServoParameters();
  }
  delay(200);
#endif




  stepper = new StepperWithLimits(stepPinStepper, dirPinStepper, minPin, maxPin);
  loadcell = new LoadCell_ADS1256();

  loadcell->setZeroPoint();
  #ifdef ESTIMATE_LOADCELL_VARIANCE
    loadcell->estimateVariance();       // automatically identify sensor noise for KF parameterization
  #endif

  // find the min & max endstops

  if (isv57LifeSignal_b && SENSORLESS_HOMING)
  {
    stepper->findMinMaxSensorless(&isv57);
  }
  else
  {
    stepper->findMinMaxEndstops();
  }

 
  Serial.print("Min Position is "); Serial.println(stepper->getLimitMin());
  Serial.print("Max Position is "); Serial.println(stepper->getLimitMax());


  // setup Kalman filter
  Serial.print("Given loadcell variance: ");
  Serial.println(loadcell->getVarianceEstimate());
  kalman = new KalmanFilter(loadcell->getVarianceEstimate());


  // setup FIR filter
  firNotchFilter = new FirNotchFilter(15);






  // initialize configuration and update local variables
  #ifdef PEDAL_IS_BRAKE
    dap_config_st.initialiseDefaults();
  #endif

  #ifdef PEDAL_IS_ACCELERATOR
    dap_config_st.initialiseDefaults_Accelerator();
  #endif

  // Load config from EEPROM, if valid, overwrite initial config
  EEPROM.begin(sizeof(DAP_config_st));
  dap_config_st.loadConfigFromEprom(dap_config_st);

  // interprete config values
  dap_calculationVariables_st.updateFromConfig(dap_config_st);

  // activate parameter update in first cycle
  configUpdateAvailable = true;
  // equalize pedal config for both tasks
  dap_config_st_local = dap_config_st;





  // setup multi tasking
  semaphore_updateJoystick = xSemaphoreCreateMutex();
  semaphore_updateConfig = xSemaphoreCreateMutex();
  semaphore_resetServoPos = xSemaphoreCreateMutex();
  delay(10);


  if(semaphore_updateJoystick==NULL)
  {
    Serial.println("Could not create semaphore");
    ESP.restart();
  }
  if(semaphore_updateConfig==NULL)
  {
    Serial.println("Could not create semaphore");
    ESP.restart();
  }



  // print all servo registers
  if (dap_config_st.payLoadPedalConfig_.debug_flags_0 & DEBUG_INFO_0_PRINT_ALL_SERVO_REGISTERS) 
  {
    if (isv57LifeSignal_b)
    {
      isv57.readAllServoParameters();
    }
  }
  



  disableCore0WDT();

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    pedalUpdateTask,   /* Task function. */
                    "pedalUpdateTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    //STACK_SIZE_FOR_TASK_1,
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
  delay(500);

  xTaskCreatePinnedToCore(
                    serialCommunicationTask,   
                    "serialCommunicationTask", 
                    10000,  
                    //STACK_SIZE_FOR_TASK_2,    
                    NULL,      
                    1,         
                    &Task2,    
                    1);     
  delay(500);

  #ifdef ISV_COMMUNICATION
    
    xTaskCreatePinnedToCore(
                      servoCommunicationTask,   
                      "servoCommunicationTask", 
                      10000,  
                      //STACK_SIZE_FOR_TASK_2,    
                      NULL,      
                      1,         
                      &Task2,    
                      1);     
    delay(500);
#endif






  

  Serial.println("Setup end");
  
}




/**********************************************************************************************/
/*                                                                                            */
/*                         Calc update function                                               */
/*                                                                                            */
/**********************************************************************************************/
void updatePedalCalcParameters()
{
  dap_calculationVariables_st.updateFromConfig(dap_config_st);
  dap_calculationVariables_st.updateEndstops(stepper->getLimitMin(), stepper->getLimitMax());
  stepper->updatePedalMinMaxPos(dap_config_st.payLoadPedalConfig_.pedalStartPosition, dap_config_st.payLoadPedalConfig_.pedalEndPosition);
  //stepper->findMinMaxLimits(dap_config_st.payLoadPedalConfig_.pedalStartPosition, dap_config_st.payLoadPedalConfig_.pedalEndPosition);
  dap_calculationVariables_st.updateStiffness();

  // tune the PID settings
  tunePidValues(dap_config_st);

  // equalize pedal config for both tasks
  dap_config_st_local = dap_config_st;
}



/**********************************************************************************************/
/*                                                                                            */
/*                         Main function                                                      */
/*                                                                                            */
/**********************************************************************************************/
void loop() {
  taskYIELD();
}


/**********************************************************************************************/
/*                                                                                            */
/*                         pedal update task                                                  */
/*                                                                                            */
/**********************************************************************************************/


//long lastCallTime = micros();
unsigned long cycleTimeLastCall = micros();
unsigned long minCyclesForFirToInit = 1000;
unsigned long firCycleIncrementer = 0;


unsigned long printCycleCounter = 0;


//void loop()
void pedalUpdateTask( void * pvParameters )
{

  for(;;){


    // system identification mode
    #ifdef ALLOW_SYSTEM_IDENTIFICATION
      if (systemIdentificationMode_b == true)
      {
        measureStepResponse(stepper, &dap_calculationVariables_st, &dap_config_st, loadcell);
        systemIdentificationMode_b = false;
      }
    #endif
    

    // controll cycle time. Delay did not work with the multi tasking, thus this workaround was integrated
    unsigned long now = micros();
    if (now - cycleTimeLastCall < PUT_TARGET_CYCLE_TIME_IN_US) // 100us = 10kHz
    {
      // skip 
      continue;
    }
    {
      // if target cycle time is reached, update last time
      cycleTimeLastCall = now;
    }

    

    // print the execution time averaged over multiple cycles
    if (dap_config_st.payLoadPedalConfig_.debug_flags_0 & DEBUG_INFO_0_CYCLE_TIMER) 
    {
      static CycleTimer timerPU("PU cycle time");
      timerPU.Bump();
    }
      

    // if a config update was received over serial, update the variables required for further computation
    if (configUpdateAvailable == true)
    {
      if(semaphore_updateConfig!=NULL)
      {

        bool configWasUpdated_b = false;
        // Take the semaphore and just update the config file, then release the semaphore
        if(xSemaphoreTake(semaphore_updateConfig, (TickType_t)1)==pdTRUE)
        {
          Serial.println("Updating pedal config");
          configUpdateAvailable = false;
          dap_config_st = dap_config_st_local;
          configWasUpdated_b = true;
          xSemaphoreGive(semaphore_updateConfig);
        }

        // update the calc params
        if (true == configWasUpdated_b)
        {
          Serial.println("Updating the calc params");
          configWasUpdated_b = false;
          dap_config_st.storeConfigToEprom(dap_config_st); // store config to EEPROM
          updatePedalCalcParameters(); // update the calc parameters
        }

      }
      else
      {
        semaphore_updateConfig = xSemaphoreCreateMutex();
        //Serial.println("semaphore_updateConfig == 0");
      }
    }



    // if reset pedal position was requested, reset pedal now
    // This function is implemented, so that in case of lost steps, the user can request a reset of the pedal psotion
    if (resetPedalPosition) {

      if (isv57LifeSignal_b && SENSORLESS_HOMING)
      {
        stepper->refindMinLimitSensorless(&isv57);
      }
      else
      {
        stepper->refindMinLimit();
      }
      
      resetPedalPosition = false;
      resetServoEncoder = true;
    }


    //#define RECALIBRATE_POSITION
    #ifdef RECALIBRATE_POSITION
      stepper->checkLimitsAndResetIfNecessary();
    #endif


      // compute pedal oscillation, when ABS is active
    float absForceOffset_fl32 = 0.0f;
    #ifdef ABS_OSCILLATION
      absForceOffset_fl32 = absOscillation.forceOffset(&dap_calculationVariables_st);
    #endif


    // compute the pedal incline angle 
    //#define COMPUTE_PEDAL_INCLINE_ANGLE
    #ifdef COMPUTE_PEDAL_INCLINE_ANGLE
      float sledPosition = sledPositionInMM(stepper);
      float pedalInclineAngle = pedalInclineAngleDeg(sledPosition, dap_config_st);

      // compute angular velocity & acceleration of incline angke
      float pedalInclineAngle_Accel = pedalInclineAngleAccel(pedalInclineAngle);

      //float legRotationalMoment = 0.0000001;
      //float forceCorrection = pedalInclineAngle_Accel * legRotationalMoment;

      //Serial.print(pedalInclineAngle_Accel);
      //Serial.println(" ");

    #endif


    // Get the loadcell reading
    float loadcellReading = loadcell->getReadingKg();

    // Do the loadcell signal filtering
    float filteredReading = kalman->filteredValue(loadcellReading, 0, dap_config_st.payLoadPedalConfig_.kf_modelNoise);
    float changeVelocity = kalman->changeVelocity();



    // Apply FIR notch filter to reduce force oscillation caused by ABS
    //#define APPLY_FIR_FILTER
    #ifdef APPLY_FIR_FILTER
      float filteredReading2 = firNotchFilter->filterValue(loadcellReading);
      if (firCycleIncrementer > minCyclesForFirToInit)
      {
        filteredReading = filteredReading2;
      }
      else
      {
          firCycleIncrementer++;
      }

      firCycleIncrementer++;
      /*if (firCycleIncrementer % 500 == 0)
      { 
        firCycleIncrementer = 0;
        Serial.print(filteredReading);
        Serial.print(",   ");
        Serial.print(filteredReading2);
        Serial.println("   ");
      }*/
      
    #endif
    

    //#define DEBUG_FILTER
    if (dap_config_st.payLoadPedalConfig_.debug_flags_0 & DEBUG_INFO_0_LOADCELL_READING) 
    {
      static RTDebugOutput<float, 2> rtDebugFilter({ "rawReading_g", "filtered_g"});
      rtDebugFilter.offerData({ loadcellReading * 1000, filteredReading * 1000});
    }
      

    /*#ifdef ABS_OSCILLATION
      filteredReading += forceAbsOffset;
    #endif*/

    // use interpolation to determine local linearized spring stiffness
    double stepperPosFraction = stepper->getCurrentPositionFraction();
    //double stepperPosFraction2 = stepper->getCurrentPositionFractionFromExternalPos( -(int32_t)(isv57.servo_pos_given_p + isv57.servo_pos_error_p - isv57.getZeroPos()) );
    //int32_t Position_Next = MoveByInterpolatedStrategy(filteredReading, stepperPosFraction, &forceCurve, &dap_calculationVariables_st, &dap_config_st);
    int32_t Position_Next = MoveByPidStrategy(filteredReading, stepperPosFraction, stepper, &forceCurve, &dap_calculationVariables_st, &dap_config_st, absForceOffset_fl32);


    //#define DEBUG_STEPPER_POS
    if (dap_config_st.payLoadPedalConfig_.debug_flags_0 & DEBUG_INFO_0_STEPPER_POS) 
    {
      static RTDebugOutput<int32_t, 5> rtDebugFilter({ "ESP_pos", "ESP_tar_pos", "ISV_pos", "frac1"});
      rtDebugFilter.offerData({ stepper->getCurrentPositionSteps(), Position_Next, -(int32_t)(isv57.servo_pos_given_p + isv57.servo_pos_error_p - isv57.getZeroPos()), (int32_t)(stepperPosFraction * 10000.)});
    }

    
    //stepper->printStates();
    

    // add dampening
    if (dap_calculationVariables_st.dampingPress  > 0.0001)
    {
      // dampening is proportional to velocity --> D-gain for stability
      Position_Next -= dap_calculationVariables_st.dampingPress * changeVelocity * dap_calculationVariables_st.springStiffnesssInv;
    }
      


    

  
    // clip target position to configured target interval
    Position_Next = (int32_t)constrain(Position_Next, dap_calculationVariables_st.stepperPosMin, dap_calculationVariables_st.stepperPosMax);

    // if pedal in min position, recalibrate position 
    #ifdef ISV_COMMUNICATION
    // Take the semaphore and just update the config file, then release the semaphore
        if(xSemaphoreTake(semaphore_resetServoPos, (TickType_t)1)==pdTRUE)
        {
          if (stepper->isAtMinPos())
          {
            stepper->correctPos(servo_offset_compensation_steps_i32);
            servo_offset_compensation_steps_i32 = 0;
          }
          xSemaphoreGive(semaphore_resetServoPos);
        }
    #endif



    // get current stepper position right before sheduling a new move
    //int32_t stepperPosCurrent = stepper->getCurrentPositionSteps();
    //int32_t stepperPosCurrent = stepper->getTargetPositionSteps();
    //int32_t movement = abs(stepperPosCurrent - Position_Next);
    //if (movement > MIN_STEPS)
    {
      stepper->moveTo(Position_Next, false);
    }

    

    // compute controller output
    if(semaphore_updateJoystick!=NULL)
    {
      if(xSemaphoreTake(semaphore_updateJoystick, (TickType_t)1)==pdTRUE) {
        joystickNormalizedToInt32 = NormalizeControllerOutputValue(loadcellReading, dap_calculationVariables_st.Force_Min, dap_calculationVariables_st.Force_Max, dap_config_st.payLoadPedalConfig_.maxGameOutput);
        //joystickNormalizedToInt32 = NormalizeControllerOutputValue(filteredReading, dap_calculationVariables_st.Force_Min, dap_calculationVariables_st.Force_Max, dap_config_st.payLoadPedalConfig_.maxGameOutput);
        xSemaphoreGive(semaphore_updateJoystick);
      }
    }
    else
    {
      semaphore_updateJoystick = xSemaphoreCreateMutex();
      //Serial.println("semaphore_updateJoystick == 0");
    }

    #ifdef PRINT_USED_STACK_SIZE
      unsigned int temp2 = uxTaskGetStackHighWaterMark(nullptr);
      Serial.print("PU task stack size="); Serial.println(temp2);
    #endif
  }
}

  








/**********************************************************************************************/
/*                                                                                            */
/*                         communication task                                                 */
/*                                                                                            */
/**********************************************************************************************/
// https://www.tutorialspoint.com/cyclic-redundancy-check-crc-in-arduino
uint16_t checksumCalculator(uint8_t * data, uint16_t length)
{
   uint16_t curr_crc = 0x0000;
   uint8_t sum1 = (uint8_t) curr_crc;
   uint8_t sum2 = (uint8_t) (curr_crc >> 8);
   int index;
   for(index = 0; index < length; index = index+1)
   {
      sum1 = (sum1 + data[index]) % 255;
      sum2 = (sum2 + sum1) % 255;
   }
   return (sum2 << 8) | sum1;
}




int32_t joystickNormalizedToInt32_local = 0;
void serialCommunicationTask( void * pvParameters )
{

  for(;;){

    // average cycle time averaged over multiple cycles 
    if (dap_config_st.payLoadPedalConfig_.debug_flags_0 & DEBUG_INFO_0_CYCLE_TIMER) 
    {
      static CycleTimer timerSC("SC cycle time");
      timerSC.Bump();
    }

    uint16_t crc;





    // read serial input 
    byte n = Serial.available();

  
    
    if (n > 0)
    {
      switch (n) {

        // likely config structure 
        case sizeof(DAP_config_st):
            
            if(semaphore_updateConfig!=NULL)
            {
              if(xSemaphoreTake(semaphore_updateConfig, (TickType_t)1)==pdTRUE)
              {
                DAP_config_st * dap_config_st_local_ptr;
                dap_config_st_local_ptr = &dap_config_st_local;
                Serial.readBytes((char*)dap_config_st_local_ptr, sizeof(DAP_config_st));

                

                // check if data is plausible
                bool structChecker = true;
                if ( dap_config_st_local.payLoadHeader_.payloadType != DAP_PAYLOAD_TYPE_CONFIG ){ 
                  structChecker = false;
                  Serial.print("Payload type expected: ");
                  Serial.print(DAP_PAYLOAD_TYPE_CONFIG);
                  Serial.print(",   Payload type received: ");
                  Serial.println(dap_config_st_local.payLoadHeader_.payloadType);
                }
                if ( dap_config_st_local.payLoadHeader_.version != DAP_VERSION_CONFIG ){ 
                  structChecker = false;
                  Serial.print("Config version expected: ");
                  Serial.print(DAP_VERSION_CONFIG);
                  Serial.print(",   Config version received: ");
                  Serial.println(dap_config_st_local.payLoadHeader_.version);
                }
                // checksum validation
                crc = checksumCalculator((uint8_t*)(&(dap_config_st_local.payLoadHeader_)), sizeof(dap_config_st_local.payLoadHeader_) + sizeof(dap_config_st_local.payLoadPedalConfig_));
                if (crc != dap_config_st_local.payloadFooter_.checkSum){ 
                  structChecker = false;
                  Serial.print("CRC expected: ");
                  Serial.print(crc);
                  Serial.print(",   CRC received: ");
                  Serial.println(dap_config_st_local.payloadFooter_.checkSum);
                }


                // if checks are successfull, overwrite global configuration struct
                if (structChecker == true)
                {
                  Serial.println("Updating pedal config");
                  configUpdateAvailable = true;          
                }
                xSemaphoreGive(semaphore_updateConfig);
              }
            }
          break;

        // likely action structure 
        case sizeof(DAP_actions_st) :

          DAP_actions_st dap_actions_st;
          Serial.readBytes((char*)&dap_actions_st, sizeof(DAP_actions_st));

          crc = checksumCalculator((uint8_t*)(&(dap_actions_st.payLoadHeader_)), sizeof(dap_actions_st.payLoadHeader_) + sizeof(dap_actions_st.payloadPedalAction_));
          if (crc != dap_actions_st.payloadFooter_.checkSum){ 
            Serial.print("CRC expected: ");
            Serial.print(crc);
            Serial.print(",   CRC received: ");
            Serial.println(dap_actions_st.payloadFooter_.checkSum);
          }
          else
          {

            // trigger reset pedal position
            if (dap_actions_st.payloadPedalAction_.resetPedalPos_u8)
            {
              resetPedalPosition = true;
            }

            // trigger ABS effect
            if (dap_actions_st.payloadPedalAction_.triggerAbs_u8)
            {
              absOscillation.trigger();
            }

            // trigger system identification
            if (dap_actions_st.payloadPedalAction_.startSystemIdentification_u8)
            {
              systemIdentificationMode_b = true;
            }

            // trigger return pedal position
            if (dap_actions_st.payloadPedalAction_.returnPedalConfig_u8)
            {
              DAP_config_st * dap_config_st_local_ptr;
              dap_config_st_local_ptr = &dap_config_st;
              //uint16_t crc = checksumCalculator((uint8_t*)(&(dap_config_st.payLoadHeader_)), sizeof(dap_config_st.payLoadHeader_) + sizeof(dap_config_st.payLoadPedalConfig_));
              crc = checksumCalculator((uint8_t*)(&(dap_config_st.payLoadHeader_)), sizeof(dap_config_st.payLoadHeader_) + sizeof(dap_config_st.payLoadPedalConfig_));
              dap_config_st_local_ptr->payloadFooter_.checkSum = crc;
              Serial.write((char*)dap_config_st_local_ptr, sizeof(DAP_config_st));
            }


          }

          break;

        default:

          // flush the input buffer
          while (Serial.available()) Serial.read();
          //Serial.flush();

          Serial.println("\nIn byte size: ");
          Serial.println(n);
          Serial.println("    Exp config size: ");
          Serial.println(sizeof(DAP_config_st) );
          Serial.println("    Exp action size: ");
          Serial.println(sizeof(DAP_actions_st) );

          break;  


          

      }
    }

    // transmit controller output
    if (IsControllerReady()) {
      if(semaphore_updateJoystick!=NULL)
      {
        if(xSemaphoreTake(semaphore_updateJoystick, (TickType_t)1)==pdTRUE)
        {
          joystickNormalizedToInt32_local = joystickNormalizedToInt32;
          xSemaphoreGive(semaphore_updateJoystick);
        }
        //else
        //{
          //Serial.println("semaphore_updateJoystick == 0");
        //}
      }
      SetControllerOutputValue(joystickNormalizedToInt32_local);
    }

  }
}



#ifdef ISV_COMMUNICATION


int16_t servoPos_last_i16 = 0;
int64_t timeSinceLastServoPosChange_l = 0;
int64_t timeNow_l = 0;
int64_t timeDiff = 0;

#define TIME_SINCE_SERVO_POS_CHANGE_TO_DETECT_STANDSTILL_IN_MS 200



uint64_t print_cycle_counter_u64 = 0;
void servoCommunicationTask( void * pvParameters )
{
  
  for(;;){

    if (dap_config_st.payLoadPedalConfig_.debug_flags_0 & DEBUG_INFO_0_CYCLE_TIMER) 
    {
      static CycleTimer timerServoCommunication("SC cycle time");
      timerServoCommunication.Bump();
    }

    if (isv57LifeSignal_b)
    {

        delay(20);
        //isv57.readServoStates();
        

        int32_t servo_offset_compensation_steps_local_i32 = 0;

        
        // condition 1: servo must be at halt
        // condition 2: the esp accel lib must be at halt
        bool cond_1 = false;;
        bool cond_2 = false;

        // check whether target position from ESP hasn't changed and is at min endstop position
        cond_2 = stepper->isAtMinPos();

        if (cond_2 == true)
        {
          isv57.readServoStates();
          int16_t servoPos_now_i16 = isv57.servo_pos_given_p;
          timeNow_l = millis();

//#define PRINT_SERVO_POS_EVERY_N_CYCLES
#ifdef PRINT_SERVO_POS_EVERY_N_CYCLES
          print_cycle_counter_u64++;
          // print servo pos every N cycles
          if ( (print_cycle_counter_u64 % 2000) == 0 )
          {
            Serial.println(servoPos_now_i16);
            print_cycle_counter_u64 = 0;
          }
#endif


          // check whether servo position has changed, in case, update the halt detection variable
          if (servoPos_last_i16 != servoPos_now_i16)
          {
            servoPos_last_i16 = servoPos_now_i16;
            timeSinceLastServoPosChange_l = timeNow_l;
          }

          // compute the time difference since last servo position change
          timeDiff = timeNow_l - timeSinceLastServoPosChange_l;

          // if time between last servo position is larger than a threshold, detect servo standstill 
          if ( (timeDiff > TIME_SINCE_SERVO_POS_CHANGE_TO_DETECT_STANDSTILL_IN_MS) 
            && (timeNow_l > 0) )
          {
            cond_1 = true;
          }
          else
          {
            cond_1 = false;
          }
        }


        

        // calculate zero position offset
        if (cond_1 && cond_2)
        {

          // reset encoder position, when pedal is at min position
          if (resetServoEncoder == true)
          {
            isv57.setZeroPos();
            resetServoEncoder = false;
          }

          // calculate encoder offset
          // movement to the back will reduce encoder value
          servo_offset_compensation_steps_local_i32 = (int32_t)isv57.getZeroPos() - (int32_t)isv57.servo_pos_given_p;
          // when pedal has moved to the back due to step losses --> offset will be positive 

          // since the encoder positions are defined in int16 space, they wrap at multiturn
          // to correct overflow, we apply modulo to take smallest possible deviation
          if (servo_offset_compensation_steps_local_i32 > pow(2,15)-1)
          {
            servo_offset_compensation_steps_local_i32 -= pow(2,16);
          }

          if (servo_offset_compensation_steps_local_i32 < -pow(2,15))
          {
            servo_offset_compensation_steps_local_i32 += pow(2,16);
          }
        }


        // invert the compensation wrt the motor direction
        if (MOTOR_INVERT_MOTOR_DIR)
        {
          servo_offset_compensation_steps_local_i32 *= -1;
        }


        if(semaphore_resetServoPos!=NULL)
          {

            // Take the semaphore and just update the config file, then release the semaphore
            if(xSemaphoreTake(semaphore_resetServoPos, (TickType_t)1)==pdTRUE)
            {
              servo_offset_compensation_steps_i32 = servo_offset_compensation_steps_local_i32;
              xSemaphoreGive(semaphore_resetServoPos);
            }

          }
          else
          {
            semaphore_resetServoPos = xSemaphoreCreateMutex();
            //Serial.println("semaphore_resetServoPos == 0");
          }



        if (dap_config_st.payLoadPedalConfig_.debug_flags_0 & DEBUG_INFO_0_SERVO_READINGS) 
        {
          static RTDebugOutput<int16_t, 4> rtDebugFilter({ "pos_p", "pos_error_p", "curr_per", "offset"});
          rtDebugFilter.offerData({ isv57.servo_pos_given_p, isv57.servo_pos_error_p, isv57.servo_current_percent, servo_offset_compensation_steps_i32});
        }

       

        
    }
    else
    {
      delay(1000);
    }


  }
}

#endif
