#pragma once

#include "DiyActivePedal_types.h"
#include "Main.h"





// see https://github.com/Dlloydev/QuickPID/blob/master/examples/PID_Basic/PID_Basic.ino
#include <QuickPID.h>

//Define Variables we'll be connecting to
float Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=0.3, Ki=50.0, Kd=0.000;
QuickPID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd,  /* OPTIONS */
               myPID.pMode::pOnError,                   /* pOnError, pOnMeas, pOnErrorMeas */
               myPID.dMode::dOnMeas,                    /* dOnError, dOnMeas */
               myPID.iAwMode::iAwOff,             /* iAwCondition, iAwClamp, iAwOff */
               myPID.Action::direct);                   /* direct, reverse */
bool pidWasInitialized = false;


/**********************************************************************************************/
/*                                                                                            */
/*                         Movement strategy: spring stiffness                                */
/*                                                                                            */
/**********************************************************************************************/

int32_t MoveByInterpolatedStrategy(float filteredLoadReadingKg, float stepperPosFraction, ForceCurve_Interpolated* forceCurve, const DAP_calculationVariables_st* calc_st, const DAP_config_st* config_st) {
  float spingStiffnessInv_lcl = calc_st->springStiffnesssInv;
  //float springStiffnessInterp = forceCurve->stiffnessAtPosition(stepperPosFraction);
  float springStiffnessInterp = forceCurve->EvalForceGradientCubicSpline(config_st, calc_st, stepperPosFraction);

  
  if (springStiffnessInterp > 0) {
    spingStiffnessInv_lcl = (1.0f / springStiffnessInterp);
  }

  // caclulate pedal position
  float pedalForceInterp = forceCurve->EvalForceCubicSpline(config_st, calc_st, stepperPosFraction);
  float stepperPosInterp = (calc_st->stepperPosMax - calc_st->stepperPosMin) * stepperPosFraction;
  return spingStiffnessInv_lcl * (filteredLoadReadingKg - pedalForceInterp) + stepperPosInterp;
}




/**********************************************************************************************/
/*                                                                                            */
/*                         Movement strategy: PID                                             */
/*                                                                                            */
/**********************************************************************************************/

void tunePidValues(DAP_config_st& config_st)
{
  myPID.SetTunings(config_st.payLoadPedalConfig_.PID_p_gain, config_st.payLoadPedalConfig_.PID_i_gain, config_st.payLoadPedalConfig_.PID_d_gain);
}

int32_t MoveByPidStrategy(float loadCellReadingKg, float stepperPosFraction, StepperWithLimits* stepper, ForceCurve_Interpolated* forceCurve, const DAP_calculationVariables_st* calc_st, DAP_config_st* config_st, float absForceOffset_fl32) {

  if (pidWasInitialized == false)
  {
    //turn the PID on
    myPID.SetTunings(Kp, Ki, Kd);
    myPID.SetMode(myPID.Control::automatic);
    pidWasInitialized = true;
    myPID.SetSampleTimeUs(PUT_TARGET_CYCLE_TIME_IN_US);
    myPID.SetOutputLimits(-1.0,0.0);

    myPID.SetTunings(config_st->payLoadPedalConfig_.PID_p_gain, config_st->payLoadPedalConfig_.PID_i_gain, config_st->payLoadPedalConfig_.PID_d_gain);
  }


  float loadCellTargetKg = forceCurve->EvalForceCubicSpline(config_st, calc_st, stepperPosFraction);
  loadCellTargetKg -=absForceOffset_fl32;
  
  // ToDO
  // - Min and Max force need to be identified from forceCurve->forceAtPosition() as they migh differ from calc_st.Force_Min & calc_st.Force_Max
  // - model predictive control, see e.g. https://www.researchgate.net/profile/Mohamed-Mourad-Lafifi/post/Model-Predictive-Control-examples/attachment/60202ac761fb570001029f61/AS%3A988637009301508%401612720839656/download/An+Introduction+to+Model-based+Predictive+Control+%28MPC%29.pdf
  //	https://www.youtube.com/watch?v=XaD8Lngfkzk
  //	https://github.com/pronenewbits/Arduino_Constrained_MPC_Library

  
  // clip to min & max force to prevent Ki to overflow
  float loadCellReadingKg_clip = constrain(loadCellReadingKg, calc_st->Force_Min, calc_st->Force_Max);
  
  // normalize input & setpoint
  //Setpoint = (loadCellReadingKg_clip - calc_st.Force_Min) / calc_st.Force_Range;
  //Input = (loadCellTargetKg - calc_st.Force_Min) / calc_st.Force_Range; 

  Setpoint = (loadCellTargetKg - calc_st->Force_Min) / calc_st->Force_Range;
  Input = (loadCellReadingKg_clip - calc_st->Force_Min) / calc_st->Force_Range; 

  // compute PID output
  myPID.Compute();
  
  // unnormalize output
  int32_t posStepperNew = -1.0 * Output * (calc_st->stepperPosMax - calc_st->stepperPosMin);//stepper->getTravelSteps();
  posStepperNew += calc_st->stepperPosMin;

  //#define PLOT_PID_VALUES
  #ifdef PLOT_PID_VALUES
    static RTDebugOutput<float, 8> rtDebugFilter({ "stepperPosFraction", "loadCellTargetKg", "loadCellReadingKg", "loadCellReadingKg_clip", "Setpoint", "Input", "Output", "posStepperNew"});
    rtDebugFilter.offerData({ stepperPosFraction, loadCellTargetKg, loadCellReadingKg, loadCellReadingKg_clip, Setpoint, Input, Output, posStepperNew});       
  #endif
  

  return posStepperNew;
}





int32_t mpcBasedMove(float loadCellReadingKg, float stepperPosFraction, StepperWithLimits* stepper, ForceCurve_Interpolated* forceCurve, const DAP_calculationVariables_st* calc_st, DAP_config_st* config_st, float absForceOffset_fl32) 
{



  static const float MOVE_MM_FOR_1KG = 3.0;
  static const float MOVE_STEPS_FOR_1KG = (MOVE_MM_FOR_1KG / TRAVEL_PER_ROTATION_IN_MM) * STEPS_PER_MOTOR_REVOLUTION;



  // get target force at current location
  float loadCellTargetKg = forceCurve->EvalForceCubicSpline(config_st, calc_st, stepperPosFraction);
  loadCellTargetKg -=absForceOffset_fl32;

  

  // get loadcell reading
  float loadCellReadingKg_clip = constrain(loadCellReadingKg, calc_st->Force_Min, calc_st->Force_Max);

  
  
  // if target force at location is lower than loadcell reading --> move towards the foot k_f * n_steps

  // Take into account system constraints like stepper rpm & acceleration

  

  // if target force at location is lower than loadcell reading --> move away from the foot -k_f * n_steps

  

  // predict target force at new location and compare to predicted force --> compute cost matrix

  
  

  

  

  // e_k = r^2 = (F_lc - k * (delta_x_0) - F_t(x_0 + delta_x_0))^2

  // r: force residue

  // e: cost

  // F_lc: current loadcell measurement

  // k: sping stiffness of the foot

  // x_0: current stepper position

  // x_1: next stepper pos

  // delta_x_0 = x_1 - x_0: step update at time step 0

  // F_t(x): target force at location

  

  

  // minimize e with x_1

  // d[e(delta_x_0)] / d[delta_x_0] == 0

  

  // d[e] / d[delta_x_0] = d[e] / d[r] * d[r] / d[delta_x_0]

  // d[e] / d[r] = 2 * r

  

  // d[r] / d[delta_x_0] = d[F_lc - k * (delta_x_0) - F_t(x_0 + delta_x_0)] = -k  - d[F_t]/d[delta_x_0]

  

  

  // MPC: sum up over planing horizon and optimize costs

  // take only the first control value & repeat in the next cycle

  // constraint |delta_x_0| < max step rate

  

  // l = sum_k( e_k(delta_x_k, x_0) )

  // where k = [0, 1, ..., N]

  
}



// see https://pidtuner.com
void measureStepResponse(StepperWithLimits* stepper, const DAP_calculationVariables_st* calc_st, const DAP_config_st* config_st, const LoadCell_ADS1256* loadcell)
{

  int32_t currentPos = stepper->getCurrentPositionSteps();
  int32_t minPos = currentPos - dap_calculationVariables_st.stepperPosRange * 0.05;
  int32_t maxPos = currentPos + dap_calculationVariables_st.stepperPosRange * 0.05;

  stepper->moveTo(minPos, true);

  Serial.println("======================================");
  Serial.println("Start system identification data:");

  unsigned long initialTime = micros();
  unsigned long t = micros();
  bool targetPosHasBeenSet_b = false;
  float loadcellReading;

  int32_t targetPos;

  for (uint32_t cycleIdx = 0; cycleIdx < 5; cycleIdx++)
  {
    // toogle target position
    if (cycleIdx % 2 == 0)
    {
      targetPos = maxPos;
    }
    else
    {
      targetPos = minPos;
    }

    targetPos = (int32_t)constrain(targetPos, dap_calculationVariables_st.stepperPosMin, dap_calculationVariables_st.stepperPosMax);

    // execute move to target position and meaure system response
    float currentPos;
    for (uint32_t sampleIdx_u32 = 0; sampleIdx_u32 < 2000; sampleIdx_u32++)
    {
      // get loadcell reading
      loadcellReading = loadcell->getReadingKg();

      // update time
      t = micros() - initialTime;

      // after some time, set target position
      if (sampleIdx_u32 == 50)
      {
        stepper->moveTo(targetPos, false);
      }

      // get current position
      currentPos = stepper->getCurrentPositionFraction();
      loadcellReading = (loadcellReading - calc_st->Force_Min) / calc_st->Force_Range; 

      static RTDebugOutput<float, 3, 9> rtDebugFilter;
      rtDebugFilter.offerData({ ((float)t) *1e-6 , currentPos,  loadcellReading});   
    }
  }

  Serial.println("======================================");
  Serial.println("End system identification data");
}
