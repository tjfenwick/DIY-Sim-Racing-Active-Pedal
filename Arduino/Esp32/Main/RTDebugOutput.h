#pragma once

#include <array>


template <typename TVALUE, int NVALS, int FLOAT_PRECISION=6>
class RTDebugOutput {
private:
  QueueHandle_t _queue_data;
  std::array<String,NVALS> _outNames;
  
public:
  RTDebugOutput(std::array<String,NVALS> outNames = {})
    : _outNames(outNames)
  {
    _queue_data = xQueueCreate(1, sizeof(std::array<TVALUE,NVALS>));
    xTaskCreatePinnedToCore(this->debugOutputTask, "debugOutputTask", 5000, this, 1, NULL, 1);
  }

  void offerData(std::array<TVALUE,NVALS> values) {
    xQueueSend(_queue_data, &values, /*xTicksToWait=*/0);
  }
  

  template <typename T>
  void printValue(String name, T value) {
    if (name.length() > 0) {
      Serial.print(name); Serial.print(":"); 
    }
    Serial.print(value); Serial.print(",");
  }
  void printValue(String name, float value) {
    if (name.length() > 0) {
      Serial.print(name); Serial.print(":"); 
    }
    Serial.print(value, FLOAT_PRECISION); Serial.print(",");
  }

  void printData() {
    std::array<TVALUE,NVALS> values;
    if (pdTRUE == xQueueReceive(_queue_data, &values, /*xTicksToWait=*/0)) {
        static SemaphoreHandle_t semaphore_print = xSemaphoreCreateMutex();
        if (xSemaphoreTake(semaphore_print, /*xTicksToWait=*/10) == pdTRUE) {
          for (int i=0; i<NVALS; i++) {
            printValue(_outNames[i], values[i]);
          }
          Serial.println(" ");
          xSemaphoreGive(semaphore_print);
        }
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
