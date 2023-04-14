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

// Initialize ADS1256 object
ADS1256 adc(7.68, 2.5, true ); // clockSpeed in Mhz,  VREF in volt, if use RESET PIN 

void setup()
{
  Serial.begin(9600);
  
  // Sending SDATAC to stop reading contionus data, so we can send other command
  adc.sendCommand(SDATAC);
  Serial.println("SDATAC command sent");
  
/*  Single Ended Mode
    ADS1256 support 8 single ended channel
    use setChannel(p) for this purpose, where p is the number of positive input channel between 0 and 7 (AIN0 to AIN7).
    AINCOM are automatically set as the negative input channel.
*/

  Serial.println("Changing channel for single ended mode.");

  for (int i = 0; i < 8; ++i)
  {
    adc.waitDRDY();
    adc.setChannel(i);
    Serial.print("Current Channel: ");
    Serial.println(adc.readRegister(MUX),HEX); // Read the multiplex register to see the current active channel
    //Should it be? adc.readCurrentChannel()
  }

/*  Differential Mode
    ADS1256 support 4 differential channel
    use setChannel(p,n) for this purpose, where
      p is the number of positive input channel between 0 and 7 (AIN0 to AIN7),
      n is the number of negative input channel between 0 and 7 (AIN0 to AIN7).
*/

  Serial.println("Changing channel for differential mode.");

  for (int i = 0; i < 8; i+=2)
  {
    adc.waitDRDY();
    adc.setChannel(i,i+1);
    Serial.print("Current Channel: ");
    Serial.println(adc.readRegister(MUX),HEX); // Read the multiplex register to see the current active channel
    //Should it be? adc.readCurrentChannel()
  }

  // Please note that AINCOM is defined as channel number 8
  // When you read the serial output,
  // 08h means AIN0 - AINCOM
  // 18h means AIN1 - AINCOM
  // 28h means AIN2 - AINCOM
  // etc

}

void loop()
{

}
