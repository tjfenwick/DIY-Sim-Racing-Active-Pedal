#pragma once

#include "DiyActivePedal_types.h"


static const long ABS_ACTIVE_TIME_PER_TRIGGER_MILLIS = 100;

class ABSOscillation {
private:
  long _timeLastTriggerMillis;

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
    long timeSinceTrigger = (timeNowMillis - _timeLastTriggerMillis);
    
    bool absActive = timeSinceTrigger < ABS_ACTIVE_TIME_PER_TRIGGER_MILLIS;
    if (absActive) {
      float secondsSinceTrigger = timeSinceTrigger / 1000.0f;
      return calcVars_st.absAmplitude * sin(calcVars_st.absFrequency * secondsSinceTrigger);
    } else {
      return 0;
    }
  }
};
