#pragma once

#include "freertos/timers.h"


static const int MAX_CYCLES = 1000;

class CycleTimer {
private:
  String _timerName;
  unsigned long _timeFirst;
  unsigned int _cycleCount;

public:
  CycleTimer(String timerName)
    : _timerName(timerName)
  {
    ResetTimer();
  }

  void ResetTimer() {
    _timeFirst = esp_timer_get_time();//micros();
    _cycleCount = 0;
  }

  void Bump() {
    _cycleCount++;
    if (_cycleCount > MAX_CYCLES) {

      ;
      unsigned long timeEnd = esp_timer_get_time();//micros();
      unsigned long timeElapsed = timeEnd - _timeFirst;
              
      double averageCycleTime = ((double)timeElapsed) / ((double)MAX_CYCLES); 
      Serial.print(_timerName); Serial.print(": "); Serial.println(averageCycleTime);

      ResetTimer();
    }
  }
};
