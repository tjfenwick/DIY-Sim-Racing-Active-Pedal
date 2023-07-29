#pragma once

#include "Arduino.h"

#if CONFIG_IDF_TARGET_ESP32S2
  #define USB_JOYSTICK
#elif CONFIG_IDF_TARGET_ESP32
  #define BLUETOOTH_GAMEPAD
#endif


void SetupController();
bool IsControllerReady();

void SetControllerOutputValue(int32_t value);
int32_t NormalizeControllerOutputValue(float value, float minVal, float maxVal, float maxGameOutput);
