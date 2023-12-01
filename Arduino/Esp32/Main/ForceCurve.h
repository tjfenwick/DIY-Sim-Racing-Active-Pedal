#pragma once

#include "DiyActivePedal_types.h"

// The number of segments, which are defined for the spline
#define NUMBER_OF_SPLINE_SEGMENTS 5

class ForceCurve_Interpolated {

public:
  float EvalForceCubicSpline(const DAP_config_st* config_st, const DAP_calculationVariables_st* calc_st, float fractionalPos);
  float EvalForceGradientCubicSpline(const DAP_config_st* config_st, const DAP_calculationVariables_st* calc_st, float fractionalPos, bool normalized_b);

};
