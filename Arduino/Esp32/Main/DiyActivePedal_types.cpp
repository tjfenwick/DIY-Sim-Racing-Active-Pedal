#include "DiyActivePedal_types.h"
#include "Arduino.h"

#include "PedalGeometry.h"
#include "StepperWithLimits.h"

static const float ABS_SCALING = 50;

void DAP_config_st::initialiseDefaults() {
  payloadType = 100;
  version = 0;
  pedalStartPosition = 35;
  pedalEndPosition = 80;

  maxForce = 90;
  preloadForce = 1;

  relativeForce_p000 = 0;
  relativeForce_p020 = 20;
  relativeForce_p040 = 40;
  relativeForce_p060 = 60;
  relativeForce_p080 = 80;
  relativeForce_p100 = 100;

  dampingPress = 0;
  dampingPull = 0;

  absFrequency = 60;
  absAmplitude = 100.0f;

  lengthPedal_AC = 150;
  horPos_AB = 215;
  verPos_AB = 80;
  lengthPedal_CB = 200;
}


void DAP_calculationVariables_st::updateFromConfig(DAP_config_st& config_st) {
  startPosRel = ((float)config_st.pedalStartPosition) / 100.0f;
  endPosRel = ((float)config_st.pedalEndPosition) / 100.0f;

  absFrequency = 2 * PI * ((float)config_st.absFrequency);
  absAmplitude = ((float)config_st.absAmplitude)/ TRAVEL_PER_ROTATION_IN_MM * STEPS_PER_MOTOR_REVOLUTION / ABS_SCALING; // in mm

  dampingPress = ((float)config_st.dampingPress) / 400.0f;

  // update force variables
  Force_Min = ((float)config_st.preloadForce) / 10.0f;
  Force_Max = ((float)config_st.maxForce) / 10.0f;
  Force_Range = Force_Max - Force_Min;
}

void DAP_calculationVariables_st::updateEndstops(long newMinEndstop, long newMaxEndstop) {
  stepperPosMinEndstop = newMinEndstop;
  stepperPosMaxEndstop = newMaxEndstop;
  stepperPosEndstopRange = stepperPosMaxEndstop - stepperPosMinEndstop;

  stepperPosMin = stepperPosEndstopRange * startPosRel;
  stepperPosMax = stepperPosEndstopRange * endPosRel;
  stepperPosRange = stepperPosMax - stepperPosMin;
}

void DAP_calculationVariables_st::updateStiffness() {
  springStiffnesss = Force_Range / stepperPosRange;
  springStiffnesssInv = 1.0 / springStiffnesss;
}
