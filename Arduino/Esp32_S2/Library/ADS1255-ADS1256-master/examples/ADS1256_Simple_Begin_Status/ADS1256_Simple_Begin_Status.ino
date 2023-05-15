// Arduino Sample Code to use ADS1256 library
// Written by Axel Sepulveda, May 2020

#include <ADS1256.h>
#include <SPI.h>

float clockMHZ = 7.68; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference

// Construct and init ADS1256 object
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v

uint8_t ADCstatus;


void setup()
{
  Serial.begin(250000);
  
  Serial.println("Starting ADC");
  
  adc.begin(); 
  ADCstatus= adc.getStatus();
  Serial.print("ADC Started: ");
  Serial.println(ADCstatus,BIN);   // print in binary format, ADS1256 should print "110000"
}

void loop()
{ 

}
