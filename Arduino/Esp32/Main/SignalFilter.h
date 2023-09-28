#pragma once

#include <Kalman.h>

static const int Nobs = 1;      // 1 filter input:   observed value
static const int Nstate = 2;    // 2 filter outputs: change & velocity
static const int Ncom = 1; // Number of commands, u vector


class KalmanFilter {
private:
  KALMAN<Nstate, Nobs, Ncom> _K;
  unsigned long _timeLastObservation;

public:
  KalmanFilter(float varianceEstimate);

  float filteredValue(float observation, float command, uint8_t modelNoiseScaling_u8);
  float changeVelocity();
};
