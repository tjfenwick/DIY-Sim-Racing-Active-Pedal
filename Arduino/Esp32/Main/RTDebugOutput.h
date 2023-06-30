#pragma once

#include <array>


template <typename TVALUE, int NVALS>
class RTDebugOutput {
private:
  SemaphoreHandle_t _semaphore_data;
  std::array<String,NVALS> _outNames;
  std::array<TVALUE,NVALS> _outValues;
  bool _dataReady;
  
public:
  RTDebugOutput(std::array<String,NVALS> outNames)
    : _outNames(outNames)
    , _dataReady(false)
  {
    _semaphore_data = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(this->debugOutputTask, "debugOutputTask", 5000, this, 1, NULL, 1);
  }

  void offerData(std::array<TVALUE,NVALS> values) {
    if(xSemaphoreTake(_semaphore_data, 0) == pdTRUE) {
      _outValues = values;
      _dataReady = true;
      xSemaphoreGive(_semaphore_data);
    }
  }

  template <typename T>
  void printValue(String name, T value) {
    Serial.print(name); Serial.print(":"); Serial.print(value); Serial.print(",");
  }
  void printValue(String name, float value) {
    Serial.print(name); Serial.print(":"); Serial.print(value,6); Serial.print(",");
  }

  void printData() {
    if(xSemaphoreTake(_semaphore_data, 0) == pdTRUE) {
      if (_dataReady) {
        for (int i=0; i<NVALS; i++) {
          printValue(_outNames[i], _outValues[i]);
        }
        Serial.println(" ");
        _dataReady = false;
      }
      xSemaphoreGive(_semaphore_data);
    }
  }

private:
  static void debugOutputTask(void* pvParameters) {
    RTDebugOutput* debugOutput = (RTDebugOutput*) pvParameters;
    for (;;) {
      debugOutput->printData();
      taskYIELD();
    }
  }
};
