/*
        ADS1256.h - Arduino Library for communication with Texas Instrument ADS1256 ADC
        Written by Adien Akhmad, August 2015
		Modfified  Jan 2019 by Axel Sepulveda for ATMEGA328
*/

#include "ADS1256.h"
#include "Arduino.h"
#include "SPI.h"

ADS1256::ADS1256(float clockspdMhz, float vref, bool useResetPin) {
  // Set DRDY as input
  pinMode(pinDRDY, INPUT);      
  // Set CS as output
  pinMode(pinCS, OUTPUT);
  
  if (useResetPin) {
    // set RESETPIN as output
    pinMode(pinRST, OUTPUT );
    // pull RESETPIN high
    pinMode(pinRST, HIGH);
  }

  // Voltage Reference
  _VREF = vref;

  // Default conversion factor
  _conversionFactor = 1.0;

  // Start SPI on a quarter of ADC clock speed
  /*SPI.begin();
  //SPI.begin(SCK, MISO, MOSI, SS);;
  SPI.beginTransaction(
      SPISettings(clockspdMhz * 1000000 / 4, MSBFIRST, SPI_MODE1));*/
}

void ADS1256::initSpi(float clockspdMhz)
{
  // Start SPI on a quarter of ADC clock speed
  
  Serial.println("SPI init: start");
  SPI.begin();
  //SPI.begin(SCK, MISO, MOSI, SS);;

  delay(500);
  SPI.beginTransaction(
      SPISettings(clockspdMhz * 1000000 / 4, MSBFIRST, SPI_MODE1));

      Serial.println("SPI init: end");

}

void ADS1256::writeRegister(unsigned char reg, unsigned char wdata) {
  CSON();
  SPI.transfer(ADS1256_CMD_WREG | reg); // opcode1 Write registers starting from reg
  SPI.transfer(0);  // opcode2 Write 1+0 registers
  SPI.transfer(wdata);  // write wdata
  delayMicroseconds(1);              
  CSOFF();
}

unsigned char ADS1256::readRegister(unsigned char reg) {
  unsigned char readValue;
  CSON();
  SPI.transfer(ADS1256_CMD_RREG | reg); // opcode1 read registers starting from reg
  SPI.transfer(0);                  // opcode2 read 1+0 registers
  delayMicroseconds(7);              //  t6 delay (4*tCLKIN 50*0.13 = 6.5 us)    
  readValue = SPI.transfer(0);          // read registers
  delayMicroseconds(1);              //  t11 delay (4*tCLKIN 4*0.13 = 0.52 us)    
  CSOFF();
  return readValue;
  
}

void ADS1256::sendCommand(unsigned char reg) {
  CSON();
  waitDRDY();
  SPI.transfer(reg);
  delayMicroseconds(1);              //  t11 delay (4*tCLKIN 4*0.13 = 0.52 us)    
  CSOFF();
}

void ADS1256::setConversionFactor(float val) { _conversionFactor = val; }

void ADS1256::readTest() {
  unsigned char _highByte, _midByte, _lowByte;
  CSON();
  SPI.transfer(ADS1256_CMD_RDATA);
  delayMicroseconds(7);              //  t6 delay (4*tCLKIN 50*0.13 = 6.5 us)    

  _highByte = SPI.transfer(ADS1256_CMD_WAKEUP);
  _midByte = SPI.transfer(ADS1256_CMD_WAKEUP);
  _lowByte = SPI.transfer(ADS1256_CMD_WAKEUP);

  CSOFF();
}

float ADS1256::readCurrentChannel() {
  CSON();
  SPI.transfer(ADS1256_CMD_RDATA);
  delayMicroseconds(7);              //  t6 delay (4*tCLKIN 50*0.13 = 6.5 us)              
  float adsCode = read_float32();
  CSOFF();
  return ((adsCode / 0x7FFFFF) * ((2 * _VREF) / (float)_pga)) *
         _conversionFactor;
}

// Reads raw ADC data, as 32bit int
long ADS1256::readCurrentChannelRaw() {
  CSON();
  SPI.transfer(ADS1256_CMD_RDATA);
  delayMicroseconds(7);              //  t6 delay (4*tCLKIN 50*0.13 = 6.5 us)       
  long adsCode = read_int32();
  CSOFF();
  return adsCode;
}

// Call this ONLY after ADS1256_CMD_RDATA command
unsigned long ADS1256::read_uint24() {
  unsigned char _highByte, _midByte, _lowByte;
  unsigned long value;

  _highByte = SPI.transfer(0);
  _midByte  = SPI.transfer(0);
  _lowByte  = SPI.transfer(0);

  // Combine all 3-bytes to 24-bit data using byte shifting.
  value = ((long)_highByte << 16) + ((long)_midByte << 8) + ((long)_lowByte);
  return value;
}

// Call this ONLY after ADS1256_CMD_RDATA command
// Convert the signed 24bit stored in an unsigned 32bit to a signed 32bit
long ADS1256::read_int32() {
  long value = read_uint24();

  if (value & 0x00800000) { // if the 24 bit value is negative reflect it to 32bit
    value |= 0xff000000;
  }

  return value;
}

// Call this ONLY after ADS1256_CMD_RDATA command
// Cast as a float
float ADS1256::read_float32() {
  long value = read_int32();
  return (float)value;
}

// Channel switching for single ended mode. Negative input channel are
// automatically set to AINCOM
void ADS1256::setChannel(byte channel) { setChannel(channel, -1); }

// Channel Switching for differential mode. Use -1 to set input channel to
// AINCOM
void ADS1256::setChannel(byte AIN_P, byte AIN_N) {
  unsigned char MUX_CHANNEL;
  unsigned char MUXP;
  unsigned char MUXN;

  switch (AIN_P) {
    case 0:
      MUXP = ADS1256_MUXP_AIN0;
      break;
    case 1:
      MUXP = ADS1256_MUXP_AIN1;
      break;
    case 2:
      MUXP = ADS1256_MUXP_AIN2;
      break;
    case 3:
      MUXP = ADS1256_MUXP_AIN3;
      break;
    case 4:
      MUXP = ADS1256_MUXP_AIN4;
      break;
    case 5:
      MUXP = ADS1256_MUXP_AIN5;
      break;
    case 6:
      MUXP = ADS1256_MUXP_AIN6;
      break;
    case 7:
      MUXP = ADS1256_MUXP_AIN7;
      break;
    default:
      MUXP = ADS1256_MUXP_AINCOM;
  }

  switch (AIN_N) {
    case 0:
      MUXN = ADS1256_MUXN_AIN0;
      break;
    case 1:
      MUXN = ADS1256_MUXN_AIN1;
      break;
    case 2:
      MUXN = ADS1256_MUXN_AIN2;
      break;
    case 3:
      MUXN = ADS1256_MUXN_AIN3;
      break;
    case 4:
      MUXN = ADS1256_MUXN_AIN4;
      break;
    case 5:
      MUXN = ADS1256_MUXN_AIN5;
      break;
    case 6:
      MUXN = ADS1256_MUXN_AIN6;
      break;
    case 7:
      MUXN = ADS1256_MUXN_AIN7;
      break;
    default:
      MUXN = ADS1256_MUXN_AINCOM;
  }

  MUX_CHANNEL = MUXP | MUXN;

  CSON();
  writeRegister(ADS1256_RADD_MUX, MUX_CHANNEL);
  sendCommand(ADS1256_CMD_SYNC);
  sendCommand(ADS1256_CMD_WAKEUP);
  CSOFF();
}

/*
Init chip with set datarate and gain and perform self calibration
*/ 
void ADS1256::begin(unsigned char drate, unsigned char gain, bool buffenable) {
  _pga = 1 << gain;
  sendCommand(ADS1256_CMD_SDATAC);  // send out ADS1256_CMD_SDATAC command to stop continous reading mode.
  writeRegister(ADS1256_RADD_DRATE, drate);  // write data rate register   
  uint8_t bytemask = B00000111;
  uint8_t adcon = readRegister(ADS1256_RADD_ADCON);
  uint8_t byte2send = (adcon & ~bytemask) | gain;
  writeRegister(ADS1256_RADD_ADCON, byte2send);
  if (buffenable) {  
    uint8_t status = readRegister(ADS1256_RADD_STATUS);   
    bitSet(status, 1); 
    writeRegister(ADS1256_RADD_STATUS, status);
  }
  sendCommand(ADS1256_CMD_SELFCAL);  // perform self calibration
  
  waitDRDY();
  ;  // wait ADS1256 to settle after self calibration
}

/*
Init chip with default datarate and gain and perform self calibration
*/ 
void ADS1256::begin() {
  sendCommand(ADS1256_CMD_SDATAC);  // send out ADS1256_CMD_SDATAC command to stop continous reading mode.
  uint8_t status = readRegister(ADS1256_RADD_STATUS);      
  sendCommand(ADS1256_CMD_SELFCAL);  // perform self calibration  
  waitDRDY();   // wait ADS1256 to settle after self calibration
}

/*
Reads and returns STATUS register
*/ 
uint8_t ADS1256::getStatus() {
  sendCommand(ADS1256_CMD_SDATAC);  // send out ADS1256_CMD_SDATAC command to stop continous reading mode.
  return readRegister(ADS1256_RADD_STATUS); 
}



void ADS1256::CSON() {
  //PORT_CS &= ~(1 << PINDEX_CS);
  digitalWrite(pinCS, LOW);
}  // digitalWrite(_CS, LOW); }

void ADS1256::CSOFF() {
  digitalWrite(pinCS, HIGH);
  //PORT_CS |= (1 << PINDEX_CS);
}  // digitalWrite(_CS, HIGH); }

void ADS1256::waitDRDY() {
  //while (PIN_DRDY & (1 << PINDEX_DRDY));
  while (digitalRead(pinDRDY));
}

boolean ADS1256::isDRDY() {
  return !digitalRead(pinDRDY);
}	
