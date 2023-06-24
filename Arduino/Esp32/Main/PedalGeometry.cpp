#include "PedalGeometry.h"

#include "StepperWithLimits.h"


float sledPositionInMM(StepperWithLimits* stepper) {
  float currentPos = stepper->getCurrentPositionSteps();
  return (currentPos / STEPS_PER_MOTOR_REVOLUTION) * TRAVEL_PER_ROTATION_IN_MM;
}

float pedalInclineAngleDeg(float sledPositionMM, DAP_config_st& config_st) {
  // see https://de.wikipedia.org/wiki/Kosinussatz
  // A: is lower pedal pivot
  // C: is upper pedal pivot
  // B: is rear pedal pivot
  float a = config_st.lengthPedal_CB;
  float b = config_st.lengthPedal_AC;
  float c_ver = config_st.verPos_AB;
  float c_hor = config_st.horPos_AB + sledPositionMM;
  float c = sqrtf(c_ver * c_ver + c_hor * c_hor);

//#define DEBUG_PEDAL_INCLINE
#ifdef DEBUG_PEDAL_INCLINE
  Serial.print("a: ");    Serial.print(a);
  Serial.print(", b: ");  Serial.print(b);
  Serial.print(", c: ");  Serial.print(c);

  Serial.print(", sledPositionMM: ");  Serial.print(sledPositionMM);
#endif

  float nom = b*b + c*c - a*a;
  float den = 2 * b * c;
  
  float alpha = 0;
  if (abs(den) > 0.01) {
    alpha = acos( nom / den );
  }

#ifdef DEBUG_PEDAL_INCLINE
  Serial.print(", alpha1: ");  Serial.print(alpha * RAD_TO_DEG);
#endif

  // add incline due to AB incline --> result is incline realtive to horizontal 
  if (abs(c_hor)>0.01) {
    alpha += atan(c_ver / c_hor);
  }

#ifdef DEBUG_PEDAL_INCLINE
  Serial.print(", alpha2: ");  Serial.print(alpha * RAD_TO_DEG);
  Serial.println(" ");
#endif
  
  return alpha * RAD_TO_DEG;
}
