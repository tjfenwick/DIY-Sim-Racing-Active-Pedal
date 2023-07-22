#pragma once

#include "freertos/timers.h"
#include "RTDebugOutput.h"

static const int MAX_CYCLES = 1000;

class CycleTimer {
private:
  RTDebugOutput<float, 1> _rtOutput;
  int64_t _timeFirst;
  unsigned int _cycleCount;

public:
  CycleTimer(String timerName)
    : _rtOutput({ timerName })
  {
    ResetTimer();
  }

  void ResetTimer() {
    _timeFirst = esp_timer_get_time();
    _cycleCount = 0;
  }

  void Bump() {
    _cycleCount++;
    if (_cycleCount > MAX_CYCLES) {
      int64_t timeEnd = esp_timer_get_time();
      int64_t timeElapsed = timeEnd - _timeFirst;
              
      float averageCycleTime = float(timeElapsed) / MAX_CYCLES;
      _rtOutput.offerData({ averageCycleTime });

      ResetTimer();
    }
  }
};
