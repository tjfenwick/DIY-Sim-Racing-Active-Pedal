/*
 * Run an example of Kalman filter.
 * This example simulates a sinusoidal position of an object.
 * The 'SIMULATOR_' functions below simulate the physical process and its measurement with sensors. 
 * Results are printed on Serial port. You can use 'kalman_full.py' to analyse them with Python.
 * 
 * Author:
 *  R.JL. Fétick
 *  
 * Revision:
 *  31 Aug 2019 - Creation
 * 
 */

#include <Kalman.h>
using namespace BLA;

//------------------------------------
/****  MODELIZATION PARAMETERS  ****/
//------------------------------------

#define Nstate 3 // position, speed, acceleration
#define Nobs 2   // position, acceleration

// measurement std of the noise
#define n_p 0.3 // position measurement noise
#define n_a 5.0 // acceleration measurement noise

// model std (1/inertia)
#define m_p 0.1
#define m_s 0.1
#define m_a 0.8

BLA::Matrix<Nobs> obs; // observation vector
KALMAN<Nstate,Nobs> K; // your Kalman filter
unsigned long T; // current time
float DT; // delay between two updates of the filter

// Note: I made 'obs' a global variable so memory is allocated before the loop.
//       This might provide slightly better speed efficiency in loop.


//------------------------------------
/****    SIMULATOR PARAMETERS   ****/
//------------------------------------

// These variables simulate a physical process to be measured
// In real life, the SIMULATOR is replaced by your operational system

BLA::Matrix<Nstate> state; // true state vector

#define SIMUL_PERIOD 0.3 // oscillating period [s]
#define SIMUL_AMP 1.0    // oscillation amplitude
#define LOOP_DELAY 10    // add delay in the measurement loop [ms]

//------------------------------------
/****        SETUP & LOOP       ****/
//------------------------------------

void setup() {

  Serial.begin(57600);

  // time evolution matrix (whatever... it will be updated inloop)
  K.F = {1.0, 0.0, 0.0,
		 0.0, 1.0, 0.0,
         0.0, 0.0, 1.0};

  // measurement matrix n the position (e.g. GPS) and acceleration (e.g. accelerometer)
  K.H = {1.0, 0.0, 0.0,
         0.0, 0.0, 1.0};
  // measurement covariance matrix
  K.R = {n_p*n_p,   0.0,
           0.0, n_a*n_a};
  // model covariance matrix
  K.Q = {m_p*m_p,     0.0,     0.0,
             0.0, m_s*m_s,     0.0,
			 0.0,     0.0, m_a*m_a};
  
  T = millis();
  
  // INITIALIZE SIMULATION
  SIMULATOR_INIT();
  
}

void loop() {
	
  // TIME COMPUTATION
  DT = (millis()-T)/1000.0;
  T = millis();

  // UPDATE STATE EQUATION
  // Here we make use of the Taylor expansion on the (position,speed,acceleration)
  // position_{k+1} = position_{k} + DT*speed_{k} + (DT*DT/2)*acceleration_{k}
  // speed_{k+1}    = speed_{k} + DT*acceleration_{k}
  // acceleration_{k+1} = acceleration_{k}
  K.F = {1.0,  DT,  DT*DT/2,
		 0.0, 1.0,       DT,
         0.0, 0.0,      1.0};

  // UPDATE THE SIMULATED PHYSICAL PROCESS
  SIMULATOR_UPDATE();
  
  // SIMULATE A NOISY MEASUREMENT WITH A SENSOR
  // Result of the measurement is written into 'obs'
  SIMULATOR_MEASURE();
  
  // APPLY KALMAN FILTER
  K.update(obs);

  // PRINT RESULTS: true state, measurements, estimated state, posterior covariance
  // The most important variable for you might be the estimated state 'K.x'
  Serial << state << ' ' << obs << ' ' << K.x << ' ' << K.P << '\n';
}

//------------------------------------
/****     SIMULATOR FUNCTIONS   ****/
//------------------------------------

void SIMULATOR_INIT(){
  // Initialize stuff for the simulator
  randomSeed(analogRead(0));
  state.Fill(0.0);
  obs.Fill(0.0);
}

void SIMULATOR_UPDATE(){
  // Simulate a physical process
  // Here we simulate a sinusoidal position of an object
  unsigned long tcur = millis();
  state(0) = SIMUL_AMP*sin(tcur/1000.0/SIMUL_PERIOD); // position
  state(1) = SIMUL_AMP/SIMUL_PERIOD*cos(tcur/1000.0/SIMUL_PERIOD); // speed
  state(2) = -SIMUL_AMP/SIMUL_PERIOD/SIMUL_PERIOD*sin(tcur/1000.0/SIMUL_PERIOD); // acceleration
}

void SIMULATOR_MEASURE(){
  // Simulate a noisy measurement of the physical process
  BLA::Matrix<Nobs> noise;
  noise(0) = n_p * SIMULATOR_GAUSS_NOISE();
  noise(1) = n_a * SIMULATOR_GAUSS_NOISE();
  obs = K.H * state + noise; // measurement
  delay(LOOP_DELAY); //simulate a delay in the measurement
}

double SIMULATOR_GAUSS_NOISE(){
  // Generate centered reduced Gaussian random number with Box-Muller algorithm
  double u1 = random(1,10000)/10000.0;
  double u2 = random(1,10000)/10000.0;
  return sqrt(-2*log(u1))*cos(2*M_PI*u2);
}