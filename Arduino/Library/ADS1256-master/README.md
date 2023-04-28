# ADS1256
Arduino Library for Texas Instrument ADS1256, working with Arduino IDE 1.8.5, Arduino UNO 

# Installation
As any arduino library (import library)

# Wiring
ADS Board   -     Arduino UNO Board

5V          -     5V

GND         -     GND

SCLK        -     pin 13 (SCK)

DIN         -     pin 11 (MOSI)

DOUT        -     pin 12 (MISO)

DRDY        -     pin 9

CS          -     pin 10

POWN       -      5V


# Examples
Basic_Switching_Channel: How to change the channels

ADS1256_Efficient_Input_Cycling: Read and print 

# Functions

ADS1256 adc(clockSpeed in Mhz, VREF in volt, boolean if use RESET PIN ): Class Constructor: 

adc.sendCommand(SDATAC): Send SDATAC to stop reading contionus data, so we can send other command

adc.waitDRDY(): Waits for the data ready flag (ADC conversion ended and ready to be read)

adc.setChannel(x): Sets the channels to be read in next reading cycle, configured in "single end" (reads between the "x" input AINx and ground GND)
  
adc.setChannel(i,j): Sets the channels to be read in next reading cycle, configured in "differential mode", first argument (i) is the positive input and second argument (j) is negative input to ADC. As stated in device datasheet any combination (i,j) (between 0 and 7) is possible but it's recomended to use adjacent inputs (0 and 1, 2 and 3, etc) 

adc.readRegister(reg): Returns value of the register "reg" 

adc.readCurrentChannel(): Returns the value of the current reading stored in ADC

#Notice: Working as 5/1/2019

