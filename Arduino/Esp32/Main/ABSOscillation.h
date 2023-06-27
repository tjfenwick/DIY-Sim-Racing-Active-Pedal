#pragma once

#include "DiyActivePedal_types.h"


static const long ABS_ACTIVE_TIME_PER_TRIGGER_MILLIS = 100;

class ABSOscillation {
private:
  long _timeLastTriggerMillis;
  long _absTimeMillis;
  long _lastCallTimeMillis = 0;

public:
  ABSOscillation()
    : _timeLastTriggerMillis(0)
  {}

public:
  void trigger() {
    _timeLastTriggerMillis = millis();
  }
  
  int32_t stepperOffset(DAP_calculationVariables_st& calcVars_st) {


    long timeNowMillis = millis();
    float timeSinceTrigger = (timeNowMillis - _timeLastTriggerMillis);
    float absStepperOffset = 0;

    if (timeSinceTrigger > ABS_ACTIVE_TIME_PER_TRIGGER_MILLIS)
    {
      _absTimeMillis = 0;
      absStepperOffset = 0;
    }
    else
    {
      _absTimeMillis += timeNowMillis - _lastCallTimeMillis;
      float absTimeSeconds = _absTimeMillis / 1000.0f;
      absStepperOffset = calcVars_st.absAmplitude * sin(calcVars_st.absFrequency * absTimeSeconds);
    }

    _lastCallTimeMillis = timeNowMillis;

    return absStepperOffset;
    

  }
};
