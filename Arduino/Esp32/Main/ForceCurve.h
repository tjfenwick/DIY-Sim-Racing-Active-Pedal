#pragma once

#include "DiyActivePedal_types.h"


static const int INTERPOLATION_NUMBER_OF_SOURCE_VALUES = 6;
static const int INTERPOLATION_NUMBER_OF_TARGET_VALUES = 1000;

class ForceCurve_Interpolated {
private:
  float _stepperPos[INTERPOLATION_NUMBER_OF_TARGET_VALUES];
  float _targetValues[INTERPOLATION_NUMBER_OF_TARGET_VALUES];
  float _springStiffness[INTERPOLATION_NUMBER_OF_TARGET_VALUES];

public:
  ForceCurve_Interpolated(DAP_config_st& config_st, DAP_calculationVariables_st& calc_st);
  float EvalForceCubicSpline(DAP_config_st& config_st, const DAP_calculationVariables_st& calc_st, float fractionalPos);
  float EvalForceGradientCubicSpline(DAP_config_st& config_st, const DAP_calculationVariables_st& calc_st, float fractionalPos);
  

private:
  int fractionalPosToIndex(float fractionalPos) const;

public:
  float stepperPos(float fractionalPos) const           { return _stepperPos[fractionalPosToIndex(fractionalPos)]; }
  float forceAtPosition(float fractionalPos) const      { return _targetValues[fractionalPosToIndex(fractionalPos)]; }
  float stiffnessAtPosition(float fractionalPos) const  { return _springStiffness[fractionalPosToIndex(fractionalPos)]; }
};
