struct DAP_config_st {
  // structure identification via payload
  uint8_t payloadType;

  // variable to check if structure at receiver matched version from transmitter
  uint8_t version;

  // To check if structure is valid
  uint8_t checkSum;

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
  uint8_t absAmplitude; // In steps

  // geometric properties of the pedal
  // in mm
  uint8_t lengthPedal_AC;
  uint8_t horPos_AB;
  uint8_t verPos_AB;
  uint8_t lengthPedal_CB;
  

} ;



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
};




void update_pedal_stiffness(DAP_calculationVariables_st * dap_calculationVariables_st_ptr)
{

  dap_calculationVariables_st_ptr->stepperPosEndstopRange = dap_calculationVariables_st_ptr->stepperPosMaxEndstop - dap_calculationVariables_st_ptr->stepperPosMinEndstop;

  dap_calculationVariables_st_ptr->stepperPosMin = dap_calculationVariables_st_ptr->stepperPosEndstopRange * dap_calculationVariables_st_ptr->startPosRel;
  dap_calculationVariables_st_ptr->stepperPosMax = dap_calculationVariables_st_ptr->stepperPosEndstopRange * dap_calculationVariables_st_ptr->endPosRel;
  dap_calculationVariables_st_ptr->stepperPosRange = dap_calculationVariables_st_ptr->stepperPosMax - dap_calculationVariables_st_ptr->stepperPosMin; 

  dap_calculationVariables_st_ptr->Force_Range = dap_calculationVariables_st_ptr->Force_Max - dap_calculationVariables_st_ptr->Force_Min;

  dap_calculationVariables_st_ptr->springStiffnesss = dap_calculationVariables_st_ptr->Force_Range / dap_calculationVariables_st_ptr->stepperPosRange;
  dap_calculationVariables_st_ptr->springStiffnesssInv = 1.0 / dap_calculationVariables_st_ptr->springStiffnesss;
  

}