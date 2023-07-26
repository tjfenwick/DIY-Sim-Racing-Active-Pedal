#include "ForceCurve.h"

#include "InterpolationLib.h"


// see https://swharden.com/blog/2022-01-22-spline-interpolation/
float ForceCurve_Interpolated::EvalForceCubicSpline(DAP_config_st& config_st, const DAP_calculationVariables_st& calc_st, float fractionalPos)
{

  float splineSegment_fl32 = fractionalPos * 5;
  uint8_t splineSegment_u8 = (uint8_t)floor(splineSegment_fl32);
  
  if (splineSegment_u8 < 0){splineSegment_u8 = 0;}
  if (splineSegment_u8 > 4){splineSegment_u8 = 4;}
  float a = config_st.payLoadPedalConfig_.cubic_spline_param_a_array[splineSegment_u8];
  float b = config_st.payLoadPedalConfig_.cubic_spline_param_b_array[splineSegment_u8];

  float yOrig[6];
  yOrig[0] = config_st.payLoadPedalConfig_.relativeForce_p000;
  yOrig[1] = config_st.payLoadPedalConfig_.relativeForce_p020;
  yOrig[2] = config_st.payLoadPedalConfig_.relativeForce_p040;
  yOrig[3] = config_st.payLoadPedalConfig_.relativeForce_p060;
  yOrig[4] = config_st.payLoadPedalConfig_.relativeForce_p080;
  yOrig[5] = config_st.payLoadPedalConfig_.relativeForce_p100;

  double dx = 1.0f;
  double t = (splineSegment_fl32 - (float)splineSegment_u8) / dx;
  double y = (1 - t) * yOrig[splineSegment_u8] + t * yOrig[splineSegment_u8 + 1] + t * (1 - t) * (a * (1 - t) + b * t);

  y = calc_st.Force_Min + y / 100.0f * calc_st.Force_Range;

  return y;
}


float ForceCurve_Interpolated::EvalForceGradientCubicSpline(DAP_config_st& config_st, const DAP_calculationVariables_st& calc_st, float fractionalPos)
{

  float splineSegment_fl32 = fractionalPos * 5;
  uint8_t splineSegment_u8 = (uint8_t)floor(splineSegment_fl32);
  
  if (splineSegment_u8 < 0){splineSegment_u8 = 0;}
  if (splineSegment_u8 > 4){splineSegment_u8 = 4;}
  float a = config_st.payLoadPedalConfig_.cubic_spline_param_a_array[splineSegment_u8];
  float b = config_st.payLoadPedalConfig_.cubic_spline_param_b_array[splineSegment_u8];

  float yOrig[6];
  yOrig[0] = config_st.payLoadPedalConfig_.relativeForce_p000;
  yOrig[1] = config_st.payLoadPedalConfig_.relativeForce_p020;
  yOrig[2] = config_st.payLoadPedalConfig_.relativeForce_p040;
  yOrig[3] = config_st.payLoadPedalConfig_.relativeForce_p060;
  yOrig[4] = config_st.payLoadPedalConfig_.relativeForce_p080;
  yOrig[5] = config_st.payLoadPedalConfig_.relativeForce_p100;

  double dx = 1.0f;
  double t = (splineSegment_fl32 - (float)splineSegment_u8) / dx;
  double dy = yOrig[splineSegment_u8 + 1] - yOrig[splineSegment_u8];
  double y_prime = dy / dx + (1 - 2 * t) * (a * (1 - t) + b * t) / dx + t * (1 - t) * (b - a) / dx;

  y_prime = y_prime / 100.0f * calc_st.Force_Range;

  return y_prime;
}

ForceCurve_Interpolated::ForceCurve_Interpolated(DAP_config_st& config_st, DAP_calculationVariables_st& calc_st)
{
  double xValues[INTERPOLATION_NUMBER_OF_SOURCE_VALUES] = { 0, 20, 40, 60, 80, 100 };
  double yValues[INTERPOLATION_NUMBER_OF_SOURCE_VALUES] = { 0, 20, 40, 60, 80, 100 };

  xValues[0] = calc_st.stepperPosMin + 0.00 * calc_st.stepperPosRange;
  xValues[1] = calc_st.stepperPosMin + 0.20 * calc_st.stepperPosRange;
  xValues[2] = calc_st.stepperPosMin + 0.40 * calc_st.stepperPosRange;
  xValues[3] = calc_st.stepperPosMin + 0.60 * calc_st.stepperPosRange;
  xValues[4] = calc_st.stepperPosMin + 0.80 * calc_st.stepperPosRange;
  xValues[5] = calc_st.stepperPosMin + 1.00 * calc_st.stepperPosRange;
  
  yValues[0] = calc_st.Force_Min + ((float)config_st.payLoadPedalConfig_.relativeForce_p000) / 100.0f * calc_st.Force_Range;
  yValues[1] = calc_st.Force_Min + ((float)config_st.payLoadPedalConfig_.relativeForce_p020) / 100.0f * calc_st.Force_Range;
  yValues[2] = calc_st.Force_Min + ((float)config_st.payLoadPedalConfig_.relativeForce_p040) / 100.0f * calc_st.Force_Range;
  yValues[3] = calc_st.Force_Min + ((float)config_st.payLoadPedalConfig_.relativeForce_p060) / 100.0f * calc_st.Force_Range;
  yValues[4] = calc_st.Force_Min + ((float)config_st.payLoadPedalConfig_.relativeForce_p080) / 100.0f * calc_st.Force_Range;
  yValues[5] = calc_st.Force_Min + ((float)config_st.payLoadPedalConfig_.relativeForce_p100) / 100.0f * calc_st.Force_Range;

//#define DEBUG_FORCECURVE
#ifdef DEBUG_FORCECURVE
  for (int i = 0; i < 6; i++) {
    Serial.print(xValues[i]);    Serial.print("  ");
    Serial.print(yValues[i]);    Serial.println(" ");  

    Serial.println(" ");
    Serial.println("Interp values: ");
  }
#endif

  for (int interpStep = 0; interpStep < INTERPOLATION_NUMBER_OF_TARGET_VALUES; interpStep++) {
    double xValueSample = ((double)interpStep) / ((double)INTERPOLATION_NUMBER_OF_TARGET_VALUES);
    xValueSample = calc_st.stepperPosMin + xValueSample * calc_st.stepperPosRange;
    _stepperPos[interpStep] = xValueSample;

    _targetValues[interpStep] = Interpolation::Linear(xValues, yValues, INTERPOLATION_NUMBER_OF_SOURCE_VALUES, xValueSample, false);
    //_targetValues[interpStep] = Interpolation::SmoothStep(xValues, yValues, INTERPOLATION_NUMBER_OF_SOURCE_VALUES, xValueSample);
    //_targetValues[interpStep] = Interpolation::CatmullSpline(xValues, yValues, INTERPOLATION_NUMBER_OF_SOURCE_VALUES, xValueSample);
    //_targetValues[interpStep] = Interpolation::ConstrainedSpline(xValues, yValues, INTERPOLATION_NUMBER_OF_SOURCE_VALUES, xValueSample);

#ifdef DEBUG_FORCECURVE
    Serial.print(xValueSample);    Serial.print("   ");    Serial.println(_targetValues[interpStep]);
#endif
  }

#ifdef DEBUG_FORCECURVE
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("Stiffness: ");
  Serial.println(calc_st.springStiffnesss);
#endif

  for (uint interpStep = 0; interpStep < (INTERPOLATION_NUMBER_OF_TARGET_VALUES-1); interpStep++) {
    _springStiffness[interpStep] = abs( _targetValues[interpStep+1] - _targetValues[interpStep]);
    _springStiffness[interpStep] /=  calc_st.stepperPosRange / ( INTERPOLATION_NUMBER_OF_TARGET_VALUES-1);

#ifdef DEBUG_FORCECURVE
    Serial.print(interpStep);    Serial.print("   ");    Serial.println(_springStiffness[interpStep]);
#endif

  }
  _springStiffness[INTERPOLATION_NUMBER_OF_TARGET_VALUES-1] = _springStiffness[INTERPOLATION_NUMBER_OF_TARGET_VALUES-2];
}

int ForceCurve_Interpolated::fractionalPosToIndex(float fractionalPos) const {
  int index = fractionalPos * INTERPOLATION_NUMBER_OF_TARGET_VALUES;
  return constrain(index, 0, INTERPOLATION_NUMBER_OF_TARGET_VALUES - 1);
}
