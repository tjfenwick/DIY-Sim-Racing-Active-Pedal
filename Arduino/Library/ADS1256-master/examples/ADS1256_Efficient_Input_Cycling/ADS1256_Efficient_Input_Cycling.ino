// Arudino Sample Code to use ADS1256 library

// Efficient Input Cycling
// Reads 4 differential channels using effiecient input cycling
/* Sensor reading:
  sensor1 = differential input connected on AIN0 - AIN1
  sensor2 = differential input connected on AIN2 - AIN3
  sensor3 = differential input connected on AIN4 - AIN5
  sensor4 = differential input connected on AIN6 - AIN7
*/
// to learn further, read on datasheet page 21, figure 19 : Cycling the ADS1256 Input Multiplexer

// http://www.ti.com/lit/ds/symlink/ads1256.pdf

// Written by Adien Akhmad, August 2015
// Modfified  Jan 2019 by Axel Sepulveda for ATMEGA328


#include <ADS1256.h>
#include <SPI.h>

float clockMHZ = 7.68; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference
// Initialize ADS1256 object
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v



float sensor1, sensor2, sensor3, sensor4;


void setup()
{
  Serial.begin(9600);
  
  Serial.println("Starting ADC");

  // start the ADS1256 with data rate of 15 SPS
  // other data rates: 
  // ADS1256_DRATE_30000SPS
  // ADS1256_DRATE_15000SPS
  // ADS1256_DRATE_7500SPS
  // ADS1256_DRATE_3750SPS
  // ADS1256_DRATE_2000SPS
  // ADS1256_DRATE_1000SPS
  // ADS1256_DRATE_500SPS
  // ADS1256_DRATE_100SPS
  // ADS1256_DRATE_60SPS
  // ADS1256_DRATE_50SPS
  // ADS1256_DRATE_30SPS
  // ADS1256_DRATE_25SPS
  // ADS1256_DRATE_15SPS
  // ADS1256_DRATE_10SPS
  // ADS1256_DRATE_5SPS
  // ADS1256_DRATE_2_5SPS
  // 
  // NOTE : Data Rate vary depending on crystal frequency. Data rates listed below assumes the crystal frequency is 7.68Mhz
  //        for other frequency consult the datasheet.
  //Posible Gains 
  //ADS1256_GAIN_1 
  //ADS1256_GAIN_2 
  //ADS1256_GAIN_4 
  //ADS1256_GAIN_8 
  //ADS1256_GAIN_16 
  //ADS1256_GAIN_32 
  //ADS1256_GAIN_64 
  adc.begin(ADS1256_DRATE_15SPS,ADS1256_GAIN_1,false); 

  Serial.println("ADC Started");
  
   // Set MUX Register to AINO and AIN1 so it start doing the ADC conversion
  adc.setChannel(0,1);
}

void loop()
{ 

  // Efficient Input Cycling
  // to learn further, read on datasheet page 21, figure 19 : Cycling the ADS1256 Input Multiplexer
  
  adc.waitDRDY(); // wait for DRDY to go low before changing multiplexer register
  adc.setChannel(2,3);   // Set the MUX for differential between ch2 and 3 
  sensor1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and AIN1

  adc.waitDRDY();
  adc.setChannel(4,5);
  sensor2 = adc.readCurrentChannel(); //// DOUT arriving here are from MUX AIN2 and AIN3

  adc.waitDRDY();
  adc.setChannel(6,7);
  sensor3 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN4 and AIN5

  adc.waitDRDY();
  adc.setChannel(0,1); // switch back to MUX AIN0 and AIN1
  sensor4 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN6 and AIN7

  //print the result.
  Serial.print(sensor1,10);
  Serial.print("\t");
  Serial.print(sensor2,10);
  Serial.print("\t");
  Serial.print(sensor3,10);
  Serial.print("\t");
  Serial.println(sensor4,10);
}

