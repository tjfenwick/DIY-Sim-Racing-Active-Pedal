#include <FastAccelStepper.h>


// these are physical properties of the stepper
static const uint32_t MAXIMUM_STEPPER_RPM = 7000;     
static const uint32_t STEPS_PER_MOTOR_REVOLUTION = 1600;
static const uint32_t MAXIMUM_STEPPER_SPEED = (MAXIMUM_STEPPER_RPM * STEPS_PER_MOTOR_REVOLUTION) / 60;   // steps/s
static const int32_t MAXIMUM_STEPPER_ACCELERATION = 1e9;                                                 // steps/s²


class StepperWithLimits {
private:
  FastAccelStepper* _stepper;
  uint8_t _pinMin,   _pinMax;      // pins that limit switches attach to
  int32_t _limitMin, _limitMax;    // stepper position at limit switches
  int32_t _posMin,   _posMax;      // stepper position at min/max of travel

public:
  StepperWithLimits(uint8_t pinStep, uint8_t pinDirection, uint8_t pinMin, uint8_t pinMax);
  bool hasValidStepper() const { return NULL != _stepper; }

public:
  void findMinMaxEndstops();
  void refindMinLimit();
  void checkLimitsAndResetIfNecessary();
  void updatePedalMinMaxPos(uint8_t pedalStartPosPct, uint8_t pedalEndPosPct);

public:
  int8_t moveTo(int32_t position, bool blocking = false);

public:
  int32_t getCurrentPositionSteps() const;
  double getCurrentPositionFraction() const;
  int32_t getTargetPositionSteps() const;

public:
  int32_t getLimitMin() const { return _limitMin; }
  int32_t getLimitMax() const { return _limitMax; }
  int32_t getTravelSteps() const { return _posMax - _posMin; }
};
