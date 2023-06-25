#pragma once

static const int MAX_CYCLES = 1000;

class CycleTimer {
private:
  String _timerName;
  long _timeFirst;
  int _cycleCount;

public:
  CycleTimer(String timerName)
    : _timerName(timerName)
  {
    ResetTimer();
  }

  void ResetTimer() {
    _timeFirst = micros();
    _cycleCount = 0;
  }

  void Bump() {
    _cycleCount++;
    if (_cycleCount >= MAX_CYCLES) {
      long timeEnd = micros();
      long timeElapsed = timeEnd - _timeFirst;
      
      long averageCycleTime = timeElapsed / MAX_CYCLES; 
      Serial.print(_timerName); Serial.print(": "); Serial.println(averageCycleTime);

      ResetTimer();
    }
  }
};
