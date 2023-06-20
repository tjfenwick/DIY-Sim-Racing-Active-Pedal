// Arudino Sample Code to use ADS1256 library
// Switching channel for ADS1256. 
// This sample code writes to MUX register and reads ADC values back.
// First for loop reads all channels in single ended mode.
// Second for loop reads 4 differential channels
// The purpose of this code is to show how to use switchChannel function.
// Written by Adien Akhmad, August 2015
// Modfified  Jan 2019 by Axel Sepulveda for ATMEGA328

#include <ADS1256.h>
#include <SPI.h>

// Construct and init ADS1256 object
ADS1256 adc(7.68, 2.5, true ); // clockSpeed in Mhz,  VREF in volt, if use RESET PIN 

void setup()
{
  Serial.begin(9600);
  
  // Sending SDATAC to stop reading contionus data, so we can send other command
  adc.sendCommand(ADS1256_CMD_SDATAC);
  Serial.println("SDATAC command sent");
  
/*  Single Ended Mode
    ADS1256 support 8 single ended channel
    use setChannel(p) for this purpose, where p is the number of positive input channel between 0 and 7 (AIN0 to AIN7).
    AINCOM are automatically set as the negative input channel.
*/

  Serial.println("Single ended mode.");

  for (int i = 0; i < 8; ++i)
  {
    adc.waitDRDY();
    adc.setChannel(i);
    Serial.print("Current Channel: ");

    Serial.print(i);
    Serial.print(" MUX: ");
    Serial.print(adc.readRegister(ADS1256_RADD_MUX),BIN); // Read the Input Multiplexer Control Register to see the current active channels
    Serial.print(" ADC Value: ");
    Serial.print(adc.readCurrentChannelRaw());
    Serial.println();
  }

/*  Differential Mode
    ADS1256 support 4 differential channel
    use setChannel(p,n) for this purpose, where
      p is the number of positive input channel between 0 and 7 (AIN0 to AIN7),
      n is the number of negative input channel between 0 and 7 (AIN0 to AIN7).
*/

  Serial.println("Changing to differential mode.");

  for (int i = 0; i < 8; i+=2)
  {
    adc.waitDRDY();
    adc.setChannel(i,i+1);
    Serial.print("Current Channel: ");
    Serial.print(i);
    Serial.print(" and ");  
    Serial.print((i+1));
    Serial.print(" MUX: ");
    Serial.print(adc.readRegister(ADS1256_RADD_MUX),BIN); // Read the Input Multiplexer Control Register to see the current active channels    
    Serial.print(" ADC Value: ");      
    Serial.print(adc.readCurrentChannelRaw());
    Serial.println();

  }

  // Please note that AINCOM is defined as channel number 8
  // When you read the serial output,
  // b 0000 1xxx   means AIN0 - AINCOM
  // b 0001 1xxx means AIN1 - AINCOM
  // b 0010 1xxx means AIN2 - AINCOM
  // etc

}

void loop()
{

}
