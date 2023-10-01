#include <SoftwareSerial.h>
#include "Modbus.h"

// define modbus stuff
#define TXPIN 27 //17
#define RXPIN 26 // 16
#define MODE  5
Modbus modbus(Serial1);




// servo states register addresses
#define reg_add_position_given_p 0x0001 // checked
#define reg_add_position_feedback_p 0x0002 // checked
#define reg_add_position_error_p 0x0003 // checked
#define reg_add_command_position_given_p 0x0004 // checked
#define reg_add_position_relative_error_p 0x0005 // checked
#define reg_add_velocity_given_rpm 0x0040 // checked
#define reg_add_velocity_feedback_rpm 0x0041 // checked
#define reg_add_velocity_error_rpm 0x0042 // checked
#define reg_add_velocity_feedback_no_filt_rpm 0x0048 // checked
#define reg_add_position_command_velocity_rpm 0x0049 // checked
#define reg_add_velocity_current_given_percent 0x0080 // checked
#define reg_add_velocity_current_feedback_percent 0x0081 // checked

#define ref_cyclic_read_0 0x01F3
#define ref_cyclic_read_1 0x01F4
#define ref_cyclic_read_2 0x01F5
#define ref_cyclic_read_3 0x01F6

// servo parameter addresses
#define pr_0_00 0x0000 // reserved parameter
#define pr_1_00 0x0000 + 25 // 1st position gain
#define pr_2_00 pr_1_00 + 40 // adaptive filter mode setup
#define pr_3_00 pr_2_00 + 30 // velocity control
#define pr_4_00 pr_3_00 + 30 // velocity torque control
#define pr_5_00 pr_4_00 + 50 // extension settings
#define pr_6_00 pr_5_00 + 40 // special settings


#define slaveId 63


void setup() {
  Serial.begin(115200); // Serial to PC
  Serial1.begin(38400, SERIAL_8N1, RXPIN, TXPIN, true); // Modbus serial
  modbus.init(MODE);


  // The iSV57 has four registers (0x0191, 0x0192, 0x0193, 0x0194) in which we can write, which values we want to obtain cyclicly
  // These registers can be obtained by sending e.g. the command: 0x63, 0x03, 0x0191, target_sate, CRC
  // tell the modbus slave, which registers will be read cyclicly
  delay(1000);
  modbus.holdingRegisterWrite(slaveId, 0x0191, reg_add_position_given_p);
  delay(50);
  modbus.holdingRegisterWrite(slaveId, 0x0192, reg_add_position_error_p);
  delay(50);
  modbus.holdingRegisterWrite(slaveId, 0x0193, reg_add_velocity_current_feedback_percent);
  delay(50);
  modbus.holdingRegisterWrite(slaveId, 0x0194, reg_add_velocity_current_given_percent);
  delay(50);


  // servo config update
  modbus.holdingRegisterWrite(slaveId, pr_0_00+2, 0); // deactivate auto gain
  delay(50);
  modbus.holdingRegisterWrite(slaveId, pr_0_00+3, 10); // machine stiffness
  delay(50);
  modbus.holdingRegisterWrite(slaveId, pr_0_00+4, 80); // ratio of inertia
  delay(50);
  modbus.holdingRegisterWrite(slaveId, pr_0_00+8, 1600); // microsteps
  delay(50);
  modbus.holdingRegisterWrite(slaveId, pr_0_00+14, 500); // position deviation setup
  delay(50);
  modbus.holdingRegisterWrite(slaveId, pr_1_00+0, 20); // 1st position gain
  delay(50);
  modbus.holdingRegisterWrite(slaveId, pr_1_00+1, 20); // 1st velocity loop gain
  delay(50);
  modbus.holdingRegisterWrite(slaveId, pr_1_00+15, 0); // control switching mode
  delay(50);


  
  // store the settings to servos NVM
  if (0)
  {
    modbus.holdingRegisterWrite(slaveId, 0x019A, 0x5555); // store the settings to servos NVM
    delay(2000);
  }
  


}


byte  raw[200];
uint8_t len;




void loop() {
  delay(10);


  int16_t regArray[4];

  // read the four registers
  for (uint8_t regIdx = 0; regIdx < 4; regIdx++)
  {
    if(modbus.requestFrom(slaveId, 0x03, ref_cyclic_read_0 + regIdx,  1) > 0)
    {
      modbus.RxRaw(raw,  len);
      regArray[regIdx] = modbus.uint16(0);
    }
    delay(10);
  }
  

  

  Serial.print("Pos_given:");
  Serial.print(regArray[0]);

  Serial.print(",Pos_error:");
  Serial.print(regArray[1]);

  Serial.print(",Cur_given:");
  Serial.print(regArray[2]);

  Serial.print(",Cur_fb:");
  Serial.print(regArray[3]);

  Serial.println(" ");

}
