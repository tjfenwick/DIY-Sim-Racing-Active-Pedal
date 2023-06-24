#include "Controller.h"

static const int16_t JOYSTICK_MIN_VALUE = 0;
static const int16_t JOYSTICK_MAX_VALUE = 10000;
static const int16_t JOYSTICK_RANGE = JOYSTICK_MAX_VALUE - JOYSTICK_MIN_VALUE;

#if defined USB_JOYSTICK
  #include <Joystick_ESP32S2.h>
  
  Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   0, 0,                 // Button Count, Hat Switch Count
                   false, false, false,  // X and Y, but no Z Axis
                   false, false, false,  // No Rx, Ry, or Rz
                   false, false,         // No rudder or throttle
                   false, true, false);  // No accelerator, brake, or steering
  
  void SetupController() {
    Joystick.setBrakeRange(JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
    delay(100);
    Joystick.begin();
  }
  bool IsControllerReady() { return true; }
  void SetControllerOutputValue(int32_t value) {
    Joystick.setBrake(value);
  }


  
  
#elif defined BLUETOOTH_GAMEPAD
  #include <BleGamepad.h>

  BleGamepad bleGamepad("DiyActiveBrake", "DiyActiveBrake", 100);
  
  void SetupController() {
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_MULTI_AXIS); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setAxesMin(JOYSTICK_MIN_VALUE); // 0 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
    bleGamepadConfig.setAxesMax(JOYSTICK_MAX_VALUE); // 32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal 
    //bleGamepadConfig.setWhichSpecialButtons(false, false, false, false, false, false, false, false);
    //bleGamepadConfig.setWhichAxes(false, false, false, false, false, false, false, false);
    bleGamepadConfig.setWhichSimulationControls(false, false, false, true, false); // only brake active 
    bleGamepadConfig.setButtonCount(0);
    bleGamepadConfig.setHatSwitchCount(0);
    bleGamepadConfig.setAutoReport(false);
    bleGamepad.begin(&bleGamepadConfig);
  }

  bool IsControllerReady() { return bleGamepad.isConnected(); }

  void SetControllerOutputValue(int32_t value) {
    //bleGamepad.setBrake(value);
    bleGamepad.setAxes(value, 0, 0, 0, 0, 0, 0, 0);
    bleGamepad.sendReport();
    //Serial.println(value);
  }
  
#endif


int32_t NormalizeControllerOutputValue(float value, float minVal, float maxVal) {
  float valRange = (maxVal - minVal);
  if (abs(valRange) < 0.01) {
    return JOYSTICK_MIN_VALUE;   // avoid div-by-zero
  }
  
  float fractional = (value - minVal) / valRange;
  int32_t controller = JOYSTICK_MIN_VALUE + (fractional * JOYSTICK_RANGE);
  return constrain(controller, JOYSTICK_MIN_VALUE, JOYSTICK_MAX_VALUE);
}
