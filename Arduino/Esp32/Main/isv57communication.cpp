#include "isv57communication.h"


// declare variables
byte  raw[200];
uint8_t len;



// initialize the communication
void isv57communication::init()
{
  Modbus modbus(Serial1);
  Serial1.begin(38400, SERIAL_8N1, RXPIN, TXPIN, true); // Modbus serial
  modbus.init(MODE);
}


// send tuned servo parameters
void isv57communication::setupServoStateReading() {
  
  // The iSV57 has four registers (0x0191, 0x0192, 0x0193, 0x0194) in which we can write, which values we want to obtain cyclicly
  // These registers can be obtained by sending e.g. the command: 0x63, 0x03, 0x0191, target_sate, CRC
  // tell the modbus slave, which registers will be read cyclicly
  modbus.holdingRegisterWrite(slaveId, 0x0191, reg_add_position_given_p);
  delay(50);
  modbus.holdingRegisterWrite(slaveId, 0x0192, reg_add_position_error_p);
  delay(50);
  modbus.holdingRegisterWrite(slaveId, 0x0193, reg_add_velocity_current_feedback_percent);
  delay(50);
  modbus.holdingRegisterWrite(slaveId, 0x0194, reg_add_velocity_current_given_percent);
  delay(50);

}



// send tuned servo parameters
void isv57communication::sendTunedServoParameters() {
  
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




// read servo states
void isv57communication::readServoStates() {

  // read the four registers
  for (uint8_t regIdx = 0; regIdx < 4; regIdx++)
  {
    regArray[regIdx] = modbus.holdingRegisterRead(slaveId, ref_cyclic_read_0 + regIdx,  2);

    if(modbus.requestFrom(slaveId, 0x03, ref_cyclic_read_0 + regIdx,  1) > 0)
    {
      modbus.RxRaw(raw,  len);
      regArray[regIdx] = modbus.uint16(0);
    }
    delay(5);
  }

  // write to public variables
  servo_pos_given_p = regArray[0];
  servo_pos_error_p = regArray[1];
  servo_current_percent = regArray[2];
  

  
  // print registers
  if (0)
  {
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
  
}
