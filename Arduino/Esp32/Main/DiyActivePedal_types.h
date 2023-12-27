#pragma once

#include <stdint.h>


#define DAP_VERSION_CONFIG 110
#define DAP_PAYLOAD_TYPE_CONFIG 100

struct payloadHeader {
  
  // structure identification via payload
  uint8_t payloadType;

  // variable to check if structure at receiver matched version from transmitter
  uint8_t version;

  // store to EEPROM flag
  uint8_t storeToEeprom;

};

struct payloadPedalAction {
  uint8_t triggerAbs_u8;
  uint8_t resetPedalPos_u8;
  uint8_t startSystemIdentification_u8;
  uint8_t returnPedalConfig_u8;
};

struct payloadPedalConfig {
  // configure pedal start and endpoint
  // In percent
  uint8_t pedalStartPosition;
  uint8_t pedalEndPosition;

  // configure pedal forces
  uint8_t maxForce;
  uint8_t preloadForce;
  
  // design force vs travel curve
  // In percent
  uint8_t relativeForce_p000; 
  uint8_t relativeForce_p020;
  uint8_t relativeForce_p040;
  uint8_t relativeForce_p060;
  uint8_t relativeForce_p080;
  uint8_t relativeForce_p100;

  // parameter to configure damping
  uint8_t dampingPress;
  uint8_t dampingPull;

  // configure ABS effect 
  uint8_t absFrequency; // In Hz
  uint8_t absAmplitude; // In kg/20

  // geometric properties of the pedal
  // in mm
  uint8_t lengthPedal_AC;
  uint8_t horPos_AB;
  uint8_t verPos_AB;
  uint8_t lengthPedal_CB;
  //Simulate ABS trigger
  uint8_t Simulate_ABS_trigger;
  uint8_t Simulate_ABS_value;
  // cubic spline parameters
  float cubic_spline_param_a_array[5];
  float cubic_spline_param_b_array[5];

  // PID parameters
  float PID_p_gain;
  float PID_i_gain;
  float PID_d_gain;

  uint8_t control_strategy_b;

  // controller settings
  uint8_t maxGameOutput;

  // Kalman filter model noise
  uint8_t kf_modelNoise;

  // debug flags, sued to enable debug output
  uint8_t debug_flags_0;

  // loadcell rating in kg / 2 --> to get value in kg, muiltiply by 2
  uint8_t loadcell_rating;
};

struct payloadFooter {
  // To check if structure is valid
  uint16_t checkSum;
};


struct DAP_actions_st {
  payloadHeader payLoadHeader_;
  payloadPedalAction payloadPedalAction_;
  payloadFooter payloadFooter_; 
};


struct DAP_config_st {

  payloadHeader payLoadHeader_;
  payloadPedalConfig payLoadPedalConfig_;
  payloadFooter payloadFooter_; 
  
  
  void initialiseDefaults();
  void initialiseDefaults_Accelerator();
  void loadConfigFromEprom(DAP_config_st& config_st);
  void storeConfigToEprom(DAP_config_st& config_st);
};


struct DAP_calculationVariables_st
{
  float springStiffnesss;
  float springStiffnesssInv;
  float Force_Min;
  float Force_Max;
  float Force_Range;
  long stepperPosMinEndstop;
  long stepperPosMaxEndstop;
  long stepperPosEndstopRange;
  
  long stepperPosMin;
  long stepperPosMax;
  float stepperPosRange;
  float startPosRel;
  float endPosRel;
  float absFrequency;
  float absAmplitude;
  

  float dampingPress;

  void updateFromConfig(DAP_config_st& config_st);
  void updateEndstops(long newMinEndstop, long newMaxEndstop);
  void updateStiffness();
};
