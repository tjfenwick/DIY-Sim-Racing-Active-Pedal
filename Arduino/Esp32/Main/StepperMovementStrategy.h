#pragma once

#include "DiyActivePedal_types.h"

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
