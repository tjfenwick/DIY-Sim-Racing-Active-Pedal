#include "StepperWithLimits.h"

static const uint8_t LIMIT_TRIGGER_VALUE = LOW;                                   // does endstop trigger high or low
static const int32_t ENDSTOP_MOVEMENT = STEPS_PER_MOTOR_REVOLUTION / 100;         // how much to move between trigger checks

FastAccelStepperEngine& stepperEngine() {
  static FastAccelStepperEngine myEngine = FastAccelStepperEngine();   // this is a factory and manager for all stepper instances

  static bool firstTime = true;
  if (firstTime) {
     myEngine.init();
     firstTime = false;
  }

  return myEngine;
}



StepperWithLimits::StepperWithLimits(uint8_t pinStep, uint8_t pinDirection, uint8_t pinMin, uint8_t pinMax)
  : _pinMin(pinMin), _pinMax(pinMax)
  , _limitMin(0),    _limitMax(0)
  , _posMin(0),      _posMax(0)
{
  pinMode(pinMin, INPUT);
  pinMode(pinMax, INPUT);
  
  _stepper = stepperEngine().stepperConnectToPin(pinStep);

  // Stepper Parameters
  if (_stepper) {
    _stepper->setDirectionPin(pinDirection, false);
    _stepper->setAutoEnable(true);
    _stepper->setSpeedInHz(MAXIMUM_STEPPER_SPEED);            // steps/s
    _stepper->setAcceleration(MAXIMUM_STEPPER_ACCELERATION);  // steps/s²

#if defined(SUPPORT_ESP32_PULSE_COUNTER)
    _stepper->attachToPulseCounter(1, 0, 0);
#endif
  }
}

void StepperWithLimits::findMinMaxLimits(uint8_t pedalStartPosPct, uint8_t pedalEndPosPct) {
  if (! hasValidStepper()) return;

  int32_t setPosition = _stepper->getCurrentPosition();
  while(! (LIMIT_TRIGGER_VALUE == digitalRead(_pinMin))){
    setPosition = setPosition - ENDSTOP_MOVEMENT;
    _stepper->moveTo(setPosition, true);
  }
  
  _stepper->forceStopAndNewPosition(0);
  _stepper->moveTo(0);
  _limitMin = 0;

  setPosition = _stepper->getCurrentPosition();
  while(! (LIMIT_TRIGGER_VALUE == digitalRead(_pinMax))){
    setPosition = setPosition + ENDSTOP_MOVEMENT;
    _stepper->moveTo(setPosition, true);
  }

  _limitMax = _stepper->getCurrentPosition();
  
  updatePedalMinMaxPos(pedalStartPosPct, pedalEndPosPct);

  _stepper->moveTo(_posMin, true);
#if defined(SUPPORT_ESP32_PULSE_COUNTER)
  _stepper->clearPulseCounter();
#endif
}

void StepperWithLimits::updatePedalMinMaxPos(uint8_t pedalStartPosPct, uint8_t pedalEndPosPct) {
  int32_t limitRange = _limitMax - _limitMin;
  _posMin = _limitMin + ((limitRange * pedalStartPosPct) / 100);
  _posMax = _limitMin + ((limitRange * pedalEndPosPct) / 100);
}

void StepperWithLimits::refindMinLimit() {
  int32_t setPosition = _stepper->getCurrentPosition();
  while(! (LIMIT_TRIGGER_VALUE == digitalRead(_pinMin))){
    setPosition = setPosition - ENDSTOP_MOVEMENT;
    _stepper->moveTo(setPosition, true);
  }
  _stepper->forceStopAndNewPosition(_limitMin);
}

void StepperWithLimits::checkLimitsAndResetIfNecessary() {
  // in case the stepper loses its position and therefore an endstop is triggered reset position
  if (LIMIT_TRIGGER_VALUE == digitalRead(_pinMin)) {
    _stepper->forceStopAndNewPosition(_limitMin);
    _stepper->moveTo(_posMin, true);
  }
  if (LIMIT_TRIGGER_VALUE == digitalRead(_pinMax)) {
    _stepper->forceStopAndNewPosition(_limitMin);
    _stepper->moveTo(_posMax, true);
  }
}

int8_t StepperWithLimits::moveTo(int32_t position, bool blocking) {
  return _stepper->moveTo(position, blocking);
}

int32_t StepperWithLimits::getCurrentPositionSteps() const {
  return _stepper->getCurrentPosition() - _posMin;
}
double StepperWithLimits::getCurrentPositionFraction() const {
  return double(getCurrentPositionSteps()) / getTravelSteps();
}

int32_t StepperWithLimits::getTargetPositionSteps() const {
  return _stepper->getPositionAfterCommandsCompleted();
}
