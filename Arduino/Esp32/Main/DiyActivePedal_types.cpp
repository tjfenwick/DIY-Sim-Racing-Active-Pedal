#include "DiyActivePedal_types.h"

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
