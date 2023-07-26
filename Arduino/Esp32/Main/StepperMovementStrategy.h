#pragma once

#include "DiyActivePedal_types.h"



// see https://github.com/br3ttb/Arduino-PID-Library/blob/master/examples/PID_Basic/PID_Basic.ino
#include <PID_v1.h>

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp=0.3, Ki=50.0, Kd=0.000;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, P_ON_E, DIRECT);
// P_ON_E: proportional on error
// P_ON_M: proportional on measurement

bool pidWasInitialized = false;




int32_t MoveByLinearStrategy(float filteredLoadReadingKg, const DAP_calculationVariables_st& calc_st) {
  float spingStiffnessInv_lcl = calc_st.springStiffnesssInv;
  // caclulate pedal position
  return spingStiffnessInv_lcl * (filteredLoadReadingKg - calc_st.Force_Min) + calc_st.stepperPosMin ;        //Calculates new position using linear function
}

int32_t MoveByInterpolatedStrategy(float filteredLoadReadingKg, float stepperPosFraction, const ForceCurve_Interpolated* forceCurve, const DAP_calculationVariables_st& calc_st) {
  float spingStiffnessInv_lcl = calc_st.springStiffnesssInv;
  float springStiffnessInterp = forceCurve->stiffnessAtPosition(stepperPosFraction);
  if (springStiffnessInterp > 0) {
    spingStiffnessInv_lcl = (1.0f / springStiffnessInterp);
  }

  // caclulate pedal position
  float pedalForceInterp = forceCurve->forceAtPosition(stepperPosFraction);
  float stepperPosInterp = forceCurve->stepperPos(stepperPosFraction);
  return spingStiffnessInv_lcl * (filteredLoadReadingKg - pedalForceInterp) + stepperPosInterp;
}



void tunePidValues(DAP_config_st& config_st)
{
  myPID.SetTunings(config_st.payLoadPedalConfig_.PID_p_gain, config_st.payLoadPedalConfig_.PID_i_gain, config_st.payLoadPedalConfig_.PID_d_gain, P_ON_E);
}

int32_t MoveByPidStrategy(float loadCellReadingKg, float stepperPosFraction, StepperWithLimits* stepper, ForceCurve_Interpolated* forceCurve, const DAP_calculationVariables_st& calc_st, DAP_config_st& config_st) {

  if (pidWasInitialized == false)
  {
    //turn the PID on
    myPID.SetMode(AUTOMATIC);
    pidWasInitialized = true;
    myPID.SetSampleTime(1);
    myPID.SetOutputLimits(0.0,1.0);

    myPID.SetTunings(config_st.payLoadPedalConfig_.PID_p_gain, config_st.payLoadPedalConfig_.PID_i_gain, config_st.payLoadPedalConfig_.PID_d_gain, P_ON_E);
  }

  float loadCellTargetKg = forceCurve->EvalForceCubicSpline(config_st, calc_st, stepperPosFraction);
  
  // ToDO
  // - Min and Max force need to be identified from forceCurve->forceAtPosition() as they migh differ from calc_st.Force_Min & calc_st.Force_Max
  // - model predictive control, see e.g. https://www.researchgate.net/profile/Mohamed-Mourad-Lafifi/post/Model-Predictive-Control-examples/attachment/60202ac761fb570001029f61/AS%3A988637009301508%401612720839656/download/An+Introduction+to+Model-based+Predictive+Control+%28MPC%29.pdf
  //	https://www.youtube.com/watch?v=XaD8Lngfkzk
  //	https://github.com/pronenewbits/Arduino_Constrained_MPC_Library

  
  // clip to min & max force to prevent Ki to overflow
  float loadCellReadingKg_clip = constrain(loadCellReadingKg, calc_st.Force_Min, calc_st.Force_Max);
  
  // normalize input & setpoint
  Setpoint = (loadCellReadingKg_clip - calc_st.Force_Min) / calc_st.Force_Range;
  Input = (loadCellTargetKg - calc_st.Force_Min) / calc_st.Force_Range; 

  // compute PID output
  myPID.Compute();
  
  // unnormalize output
  int32_t posStepperNew = 1.0 * Output * (calc_st.stepperPosMax - calc_st.stepperPosMin);//stepper->getTravelSteps();
  posStepperNew += calc_st.stepperPosMin;


  //#define PLOT_PID_VALUES
  #ifdef PLOT_PID_VALUES
    static RTDebugOutput<float, 8> rtDebugFilter({ "stepperPosFraction", "loadCellTargetKg", "loadCellReadingKg", "loadCellReadingKg_clip", "Setpoint", "Input", "Output", "posStepperNew"});
    rtDebugFilter.offerData({ stepperPosFraction, loadCellTargetKg, loadCellReadingKg, loadCellReadingKg_clip, Setpoint, Input, Output, posStepperNew});       
  #endif
  

  return posStepperNew;
}




int32_t MoveByForceTargetingStrategy(float loadCellReadingKg, StepperWithLimits* stepper, ForceCurve_Interpolated* forceCurve) {
  float stepperPosFraction = stepper->getCurrentPositionFraction();
  float loadCellTargetKg = forceCurve->forceAtPosition(stepperPosFraction);


  
  float loadCellErrorKg = loadCellReadingKg - loadCellTargetKg;

  // how many mm movement to order if 1kg of error force is detected
  // this can be tuned for responsiveness vs oscillation
  static const float MOVE_MM_FOR_1KG = 3.0;
  static const float MOVE_STEPS_FOR_1KG = (MOVE_MM_FOR_1KG / TRAVEL_PER_ROTATION_IN_MM) * STEPS_PER_MOTOR_REVOLUTION;

  // square the error to smooth minor variations
  float loadCellErrorMultipler = loadCellErrorKg;
  if (loadCellErrorKg < 0.0) {
    loadCellErrorMultipler = sq(max(-1.0f, loadCellErrorKg) / min(1.0f, loadCellTargetKg)) * -1.0;
  } else if (loadCellErrorKg < 1.0) {
    loadCellErrorMultipler = sq(loadCellErrorKg);
  }

  int32_t posStepperChange = loadCellErrorMultipler * MOVE_STEPS_FOR_1KG;
  int32_t posStepper = stepper->getCurrentPositionSteps();
  int32_t posStepperNew = posStepper + posStepperChange;

  bool overshoot = false;
  do {   // check for overshoot
    float stepperPosFractionNew = stepperPosFraction + (posStepperChange / stepper->getTravelSteps());
    float loadCellTargetKgAtPosNew = forceCurve->forceAtPosition(stepperPosFractionNew);

    overshoot = 
      (loadCellTargetKg < loadCellReadingKg && loadCellReadingKg < loadCellTargetKgAtPosNew) ||
      (loadCellTargetKg > loadCellReadingKg && loadCellReadingKg > loadCellTargetKgAtPosNew);

    if (overshoot) {
      int32_t corrected = (posStepper + posStepperNew) / 2;
      if (corrected == posStepperNew) {
        overshoot = false;
      } else {
        posStepperNew = corrected;   // and check again
      }
    }
  } while (overshoot);

  return posStepperNew;
}
