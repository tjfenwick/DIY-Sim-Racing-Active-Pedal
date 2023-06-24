long currentTime = 0;
long elapsedTime = 0;
long previousTime = 0;
float averageCycleTime = 0.0f;
uint64_t maxCycles = 1000;
uint64_t cycleIdx = 0;

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

#include "ForceCurve.h"

ForceCurve_Interpolated* forceCurve;

#define INTERP_SPRING_STIFFNESS


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
  int32_t joystickNormalizedToInt32 = 0;                           // semaphore protected data


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
#elif CONFIG_IDF_TARGET_ESP32
  #define minPin 34
  #define maxPin 35
  #define dirPinStepper    0//8
  #define stepPinStepper   4//9
#endif


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

  stepper = new StepperWithLimits(stepPinStepper, dirPinStepper, minPin, maxPin);
  loadcell = new LoadCell_ADS1256();

  loadcell->setZeroPoint();
  #ifdef ESTIMATE_LOADCELL_VARIANCE
    loadcell->estimateVariance();       // automatically identify sensor noise for KF parameterization
  #endif

  stepper->findMinMaxLimits(dap_config_st.pedalStartPosition, dap_config_st.pedalEndPosition);
  dap_calculationVariables_st.stepperPosMinEndstop = stepper->getLimitMin();
  dap_calculationVariables_st.stepperPosMaxEndstop = stepper->getLimitMax();

  Serial.print("Min Position is "); 
  Serial.println( dap_calculationVariables_st.stepperPosMinEndstop );
  Serial.print("Max Position is "); 
  Serial.println( dap_calculationVariables_st.stepperPosMaxEndstop );

  // compute pedal stiffness parameters
  update_pedal_stiffness(&dap_calculationVariables_st);
  #ifdef INTERP_SPRING_STIFFNESS
    forceCurve = new ForceCurve_Interpolated(dap_config_st, dap_calculationVariables_st);
  #endif

  kalman = new KalmanFilter(loadcell->getVarianceEstimate());


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
            delete forceCurve;
            forceCurve = new ForceCurve_Interpolated(dap_config_st, dap_calculationVariables_st);
          #endif
          xSemaphoreGive(semaphore_updateConfig);
        }
      }



      // if reset pedal position was requested, reset pedal now
      // This function is implemented, so that in case of lost steps, the user can request a reset of the pedal psotion
      if (resetPedalPosition) {
        stepper->refindMinLimit();
        resetPedalPosition = false;
      }


      //#define RECALIBRATE_POSITION
      #ifdef RECALIBRATE_POSITION
        stepper->checkLimitsAndResetIfNecessary();
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
      float sledPosition = sledPositionInMM(stepper);
      float pedalInclineAngle = pedalInclineAngleDeg(sledPosition, dap_config_st);
    #endif
    
    float loadcellReading = loadcell->getReadingKg();
    
    float filteredReading = kalman->filteredValue(loadcellReading);
    float changeVelocity = kalman->changeVelocity();
      

      // use interpolation to determine local linearized spring stiffness
      #ifndef INTERP_SPRING_STIFFNESS
        float spingStiffnessInv_lcl = dap_calculationVariables_st.springStiffnesssInv;
        // caclulate pedal position
        Position_Next = spingStiffnessInv_lcl * (filteredReading - dap_calculationVariables_st.Force_Min) + dap_calculationVariables_st.stepperPosMin ;        //Calculates new position using linear function

      #else
        double stepperPosFraction = stepper->getCurrentPositionFraction();
        
        float spingStiffnessInv_lcl = dap_calculationVariables_st.springStiffnesssInv;
        float springStiffnessInterp = forceCurve->stiffnessAtPosition(stepperPosFraction);
        if (springStiffnessInterp > 0) {
          spingStiffnessInv_lcl = (1.0f / springStiffnessInterp);
        }

        // caclulate pedal position
        float pedalForceInterp = forceCurve->forceAtPosition(stepperPosFraction);
        float stepperPosInterp = forceCurve->stepperPos(stepperPosFraction);
        Position_Next = spingStiffnessInv_lcl * (filteredReading - pedalForceInterp) + stepperPosInterp;

      #endif

      
      

      // add dampening
      if (dap_calculationVariables_st.dampingPress  > 0.0001)
      {
        // dampening is proportional to velocity --> D-gain for stability
        Position_Next -= dap_calculationVariables_st.dampingPress * changeVelocity * dap_calculationVariables_st.springStiffnesssInv;
      }
      


    #ifdef ABS_OSCILLATION
      Position_Next += stepperAbsOffset;
    #endif
      // clip target position to configured target interval
      Position_Next = (int32_t)constrain(Position_Next, dap_calculationVariables_st.stepperPosMin, dap_calculationVariables_st.stepperPosMax);


      // get current stepper position right before sheduling a new move
      //int32_t stepperPosCurrent = stepper->getCurrentPositionSteps();
      int32_t stepperPosCurrent = stepper->getTargetPositionSteps();
      long movement = abs( stepperPosCurrent - Position_Next);
      if (movement>MIN_STEPS  )
      {
        stepper->moveTo(Position_Next, false);
      }

      // compute controller output
      if(xSemaphoreTake(semaphore_updateJoystick, 1)==pdTRUE) {
        joystickNormalizedToInt32 = NormalizeControllerOutputValue(filteredReading, dap_calculationVariables_st.Force_Min, dap_calculationVariables_st.Force_Max);
        xSemaphoreGive(semaphore_updateJoystick);
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
      if (IsControllerReady()) {
        delay(1);
        if(xSemaphoreTake(semaphore_updateJoystick, 1)==pdTRUE)
        {
          int32_t joystickNormalizedToInt32_local = joystickNormalizedToInt32;
          xSemaphoreGive(semaphore_updateJoystick);
          
          SetControllerOutputValue(joystickNormalizedToInt32_local);
        }
      }

    }
  }
