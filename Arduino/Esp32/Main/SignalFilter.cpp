#include "SignalFilter.h"

// adjust model noise here s = 0.5 * a * delta_t^2 --> a = 2 * s / delta_t^2
static const float KF_MODEL_NOISE_FORCE_ACCELERATION = ( 2.0f * 9.0f / 0.05f/ 0.05f );


KalmanFilter::KalmanFilter(float varianceEstimate)
  : _timeLastObservation(micros())
{
  // evolution matrix. Size is <Nstate,Nstate>
  _K.F = {1.0, 0.0,
          0.0, 1.0};
        
  // measurement matrix. Size is <Nobs,Nstate>
  _K.H = {1.0, 0.0};

  // model covariance matrix. Size is <Nstate,Nstate>
  _K.Q = {1.0, 0.0,
          0.0, 1.0};

  // measurement covariance matrix. Size is <Nobs,Nobs>
  _K.R = { varianceEstimate };
}

float KalmanFilter::filteredValue(float observation) {
  // obtain time
  unsigned long currentTime = micros();
  unsigned long elapsedTime = currentTime - _timeLastObservation;
  if (elapsedTime < 1) { elapsedTime=1; }
  _timeLastObservation = currentTime;

  // update state transition and system covariance matrices
  float delta_t = (float)elapsedTime / 1000000.0f; // convert to seconds
  float delta_t_pow2 = delta_t * delta_t;
  float delta_t_pow3 = delta_t_pow2 * delta_t;
  float delta_t_pow4 = delta_t_pow2 * delta_t_pow2;

  _K.F = {1.0,  delta_t, 
          0.0,  1.0};

  float K_Q_11 = KF_MODEL_NOISE_FORCE_ACCELERATION * 0.5f * delta_t_pow3;
  _K.Q = {KF_MODEL_NOISE_FORCE_ACCELERATION * 0.25f * delta_t_pow4,   K_Q_11,
        K_Q_11, KF_MODEL_NOISE_FORCE_ACCELERATION * delta_t_pow2};
        

  // APPLY KALMAN FILTER
  _K.update({observation});
  return _K.x(0,0);
}

float KalmanFilter::changeVelocity() {
  return _K.x(0,1);
}
