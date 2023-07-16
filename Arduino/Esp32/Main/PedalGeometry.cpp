#include "PedalGeometry.h"

#include "StepperWithLimits.h"

KALMAN<3,1> K_pedal_geometry;
unsigned long _timeLastObservation;

static const float KF_MODEL_NOISE_FORCE_ACCELERATION = ( 10000000000. );


// evolution matrix. Size is <Nstate,Nstate>
  /*K_pedal_geometry.F = {1.0, 0.0, 0.0,
           0.0, 1.0, 0.0,
           0.0, 0.0, 1.0};*/
        
  

  




float sledPositionInMM(StepperWithLimits* stepper) {
  float currentPos = stepper->getCurrentPositionSteps();
  return (currentPos / STEPS_PER_MOTOR_REVOLUTION) * TRAVEL_PER_ROTATION_IN_MM;
}

float pedalInclineAngleDeg(float sledPositionMM, DAP_config_st& config_st) {
  // see https://de.wikipedia.org/wiki/Kosinussatz
  // A: is lower pedal pivot
  // C: is upper pedal pivot
  // B: is rear pedal pivot
  float a = config_st.payLoadPedalConfig_.lengthPedal_CB;
  float b = config_st.payLoadPedalConfig_.lengthPedal_AC;
  float c_ver = config_st.payLoadPedalConfig_.verPos_AB;
  float c_hor = config_st.payLoadPedalConfig_.horPos_AB + sledPositionMM;
  float c = sqrtf(c_ver * c_ver + c_hor * c_hor);

//#define DEBUG_PEDAL_INCLINE
#ifdef DEBUG_PEDAL_INCLINE
  Serial.print("a: ");    Serial.print(a);
  Serial.print(", b: ");  Serial.print(b);
  Serial.print(", c: ");  Serial.print(c);

  Serial.print(", sledPositionMM: ");  Serial.print(sledPositionMM);
#endif

  float nom = b*b + c*c - a*a;
  float den = 2 * b * c;
  
  float alpha = 0;
  if (abs(den) > 0.01) {
    alpha = acos( nom / den );
  }

#ifdef DEBUG_PEDAL_INCLINE
  Serial.print(", alpha1: ");  Serial.print(alpha * RAD_TO_DEG);
#endif

  // add incline due to AB incline --> result is incline realtive to horizontal 
  if (abs(c_hor)>0.01) {
    alpha += atan(c_ver / c_hor);
  }





#ifdef DEBUG_PEDAL_INCLINE
  Serial.print(", alpha2: ");  Serial.print(alpha * RAD_TO_DEG);
  Serial.println(" ");
#endif

  
  return alpha * RAD_TO_DEG;
}


float pedalInclineAngleAccel(float pedalInclineAngleDeg_global) {


  // estimate pedals angular velocity and acceleration
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

  K_pedal_geometry.F = {1.0,  delta_t, 0.5 * delta_t * delta_t,
          0.0,  1.0, delta_t,
          0.0, 0.0, 1.0};

  // measurement matrix. Size is <Nobs,Nstate>
  K_pedal_geometry.H = {1.0, 0.0, 0.0};

  // model covariance matrix. Size is <Nstate,Nstate>
  /*K_pedal_geometry.Q = {1000, 0.0, 0.0,
          0.0, 1000, 0.0,
          0.0, 0.0, 1000};*/

  // measurement covariance matrix. Size is <Nobs,Nobs>
  K_pedal_geometry.R = { 0.0001 };

  /*
  float K_Q_11 = KF_MODEL_NOISE_FORCE_ACCELERATION * 0.5f * delta_t_pow3;
  float K_Q_12 = KF_MODEL_NOISE_FORCE_ACCELERATION * 0.5f * delta_t_pow2;
  K_pedal_geometry.Q = {  KF_MODEL_NOISE_FORCE_ACCELERATION * 0.25f * delta_t_pow4,   K_Q_11,                                                   K_Q_12,
                          K_Q_11,                                                     KF_MODEL_NOISE_FORCE_ACCELERATION * delta_t_pow2,         delta_t,
                          K_Q_12,                                                     delta_t,                                                  1.0};
*/  


  // 1 * x + deltaT * x_d + 0.5 * deltaT^2 * x_dd + 1/6 * deltaT^3 * x_ddd
  // 1 / 6 * delta_t * delta_t * delta_t
  // 1 / 2 * delta_t * delta_t
  // delta_t

  float Q11 = KF_MODEL_NOISE_FORCE_ACCELERATION * (1. / 6. * delta_t * delta_t * delta_t) * (1. / 6. * delta_t * delta_t * delta_t);
  float Q12 = KF_MODEL_NOISE_FORCE_ACCELERATION * (1. / 6. * delta_t * delta_t * delta_t) * (1. / 2. * delta_t * delta_t);
  float Q13 = KF_MODEL_NOISE_FORCE_ACCELERATION * (1. / 6. * delta_t * delta_t * delta_t) * (delta_t);

  float Q21 = Q12;
  float Q22 = KF_MODEL_NOISE_FORCE_ACCELERATION * (1. / 2. * delta_t * delta_t) * (1. / 2. * delta_t * delta_t);
  float Q23 = KF_MODEL_NOISE_FORCE_ACCELERATION * (1. / 2. * delta_t * delta_t) * (delta_t);

  float Q31 = Q13;
  float Q32 = Q23;
  float Q33 = KF_MODEL_NOISE_FORCE_ACCELERATION * (delta_t) * (delta_t);





  K_pedal_geometry.Q = {  Q11, Q12, Q12,
                          Q21, Q21, Q21,
                          Q31, Q32, Q33};

        

  // APPLY KALMAN FILTER
  K_pedal_geometry.update({pedalInclineAngleDeg_global});
  float pedalPos = K_pedal_geometry.x(0,0);
  float pedalVel = K_pedal_geometry.x(0,1);
  float pedalAccel = K_pedal_geometry.x(0,2);


  //Serial.print("alpha2: ");  
  //Serial.print(pedalInclineAngleDeg_global * RAD_TO_DEG);
  //Serial.print(", pedalPos: ");  
  //Serial.print(pedalPos * RAD_TO_DEG);
  //Serial.print(", pedalVel: ");  
  //Serial.print(pedalVel);
  //Serial.print(", pedalAccel: ");  

  //Serial.print(",   ");
  //Serial.print(pedalAccel);
  //Serial.println(" ");

  return pedalAccel * RAD_TO_DEG;

 } 
