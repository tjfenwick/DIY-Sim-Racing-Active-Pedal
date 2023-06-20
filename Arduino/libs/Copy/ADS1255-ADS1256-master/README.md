<h1 align="center">
  <br>
  <a href=#><img src="https://github.githubassets.com/images/modules/logos_page/GitHub-Logo.png" width="200px" alt="banner"></a>
</h1>

<h4 align="center"> ADS1255/1256 Arduino Library

<p align="center">
     <!-- Badges Here -->
</p>
      
<p align="center">
  <a href="#about">About</a> •
  <a href="#installation">Installation</a> •  
  <a href="#usage">Usage</a> •
  <a href="#contributing">Contributing</a> •
  <a href="#credits">Credits</a> •
  <a href="#support">Support</a> •
  <a href="#license">License</a>
</p>

---

## About

Arduino Library for Texas Instrument ADS1256, working with Arduino IDE 1.8.5 
Please read datasheet: 
http://www.ti.com/lit/ds/symlink/ads1256.pdf

<table>
<tr>
<td>

![Imagen/gif de preview](#)
<p align="right">
<sub>(Preview)</sub>
</p>

</td>
</tr>
</table>


## To Do and wish List
- Add ESP8266/32 support

## Branches list
- Master: keeps sync and push to https://github.com/adienakhmad/ADS1256
- New examples: developing new examples code
- Dev: Development and testing
- DevESP: Add ESP8266 and ESP32 support  
  
  
### Features
<!--  
|                            |  theProject        | ◾ Other           |
| -------------------------- | :----------------: | :---------------: |
| Feature 1                  |         ✔️         |        ❌        |
| Feature 2                  |         ✔️         |        ❌        |-->


## Installation
  
As any arduino library with Arduino IDE (download repo, import library)

### Compatibility 
Tested with arduino UNO and MEGA
  

## Usage


# Wiring
| ADS Board | Arduino UNO Board |
|-----------|-------------------|
| * 5V      | 5V                |
| * GND     | GND               |
| * SCLK    | pin 13 (SCK)      |
| * DIN     | pin 11 (MOSI)     |
| * DOUT    | pin 12 (MISO)     |
| * DRDY    | pin 9             |
| * CS      | pin 10            |
| * POWN    | 5V                |


| ADS Board | NodeMCU v2        |
|-----------|-------------------|
| * 5V      | 5V  (vin)         |
| * GND     | GND               |
| * SCLK    | pin D5 (HSCK)     |
| * DIN     | pin D7 (HMOSI)    |
| * DOUT    | pin D6 (HMISO)    |
| * DRDY    | pin D0            |
| * CS      | pin D8 (HCS)      |
| * POWN    | 5V or 3V3         |

| ADS Board | ESP32 WROOM        |
|-----------|-------------------|
| * 5V      | 5V  (vin)         |
| * GND     | GND               |
| * SCLK    | pin D (HSCK)     |
| * DIN     | pin D (HMOSI)    |
| * DOUT    | pin D (HMISO)    |
| * DRDY    | pin D            |
| * CS      | pin D (HCS)      |
| * POWN    | 5V or 3V3         |

Programming ESP32 dev board: 
I recomend using terminal and execute command instead programming with IDE as sometimes the COM programming fails and the IDE takes long to compile and try to upload again, to enter bootload in dev boar press the boot/flash and "EN", release "EN" and then boot/flash. 
https://github.com/espressif/esptool/wiki/ESP32-Boot-Mode-Selection

## Examples

Please refer to the examples and function list below

Basic_Switching_Channel: How to read and change the channels, in single and differential ended mode: 
https://github.com/adienakhmad/ADS1256/tree/master/examples/ADS1256_Basic_Switching_Channel

ADS1256_Efficient_Input_Cycling: Read and print continuously the ADC values, changes the channel to be read in the next cycle and reads the current ADC data. (reads in differential ended mode): 
https://github.com/adienakhmad/ADS1256/tree/master/examples/ADS1256_Efficient_Input_Cycling



#### Bug Reports & Feature Requests

Please use the [issue tracker](#) to report any bugs or file feature requests.




## Support

  ## Basic Functions and methods

* ADS1256 adc(clockSpeed in Mhz, VREF in volt, boolean if use RESET PIN ): Class Constructor, specify the clockspeed (depends on your board, the reference voltage of the ADC, and if youare conecting the reset pin. 

* adc.waitDRDY(): Data ready Flag, this method waits for the data ready flag (ADC conversion ended and ready to be read). You have to read and wait for this pin signal for the ADC finish the conversion and have a correct timing and Sample Rate. This function is blocking (its a while() until Data ready pin is High), so you should read data "asyncronously" or efficiently (see example code ADS1256_Efficient_Input_Cycling). Returns void

* adc.isDRDY(): Non blocking function, checks if cycle convertion is completed and data ready to be read. Returns boolean

* adc.setChannel(x): Sets the channels to be read in next reading cycle, configured in "single end" (reads between the "x" input AINx and ground GND). Returns void
  
* adc.setChannel(i,j): Sets the channels to be read in next reading cycle, configured in "differential mode", first argument (i) is the positive input and second argument (j) is negative input to ADC. As stated in device datasheet any combination (i,j) (between 0 and 7) is possible but it's recomended to use adjacent inputs (0 and 1, 2 and 3, etc). Returns void

* adc.readCurrentChannel(): Returns the value of the currently raw data stored in the MUX register. Returns float (should be changed to long...) 

* adc.readCurrentChannelVolts(): Returns the value of the currently data stored in the MUX register converted to Volts. Returns float


### Notice: 
According to datasheet, in order to not to lose the previous and new data, it is stored in MUX register and works indepently of the new ADC data channel, every time you change the channel you have to wait for data ready signal for the new channel data to be available to be read. 

Take this sequence as Example: 

1. adc.setChannel(2); // will set CH2 to be read in next cycle
2. adc.waitDRDY(); // will wait until CH2 ADC data is filled in the MUX register
3. adc.setChannel(3); // will set CH3 to be read in next cycle (data of CH2 will be read next)
4. adc.readCurrentChannel(); // will read content un the MUX, which is CH2 data 
5. adc.waitDRDY(); // will wait until CH3 ADC data is filled in the MUX register
6. adc.readCurrentChannel(); // will read content un the MUX, which is CH3 data

This way code and ADC may work faster, as you can do other stuff while ADC reads

## Advanced functions

If you want to exploit all the features of the chip (http://www.ti.com/lit/ds/symlink/ads1256.pdf) you can use these functions to read/write the registers 

* adc.sendCommand(command): sends a valid "command" to the ADC. Refer to datasheet http://www.ti.com/lit/ds/symlink/ads1256.pdf for further info, page34. Valid comands: 
  * WAKEUP Completes SYNC and Exits Standby Mode
  * RDATA Read Data
  * RDATAC Read Data Continuously
  * SDATAC Stop Read Data Continuously
  * RREG Read from REG rrr
  * WREG Write to REG rrr
  * SELFCAL Offset and Gain Self-Calibration
  * SELFOCAL Offset Self-Calibration
  * SELFGCAL Gain Self-Calibration
  * SYSOCAL System Offset Calibration
  * SYSGCAL System Gain Calibration
  * SYNC Synchronize the A/D Conversion
  * STANDBY Begin Standby Mode
  * RESET Reset to Power-Up Values
  * WAKEUP Completes SYNC and Exits Standby Mode

Example: 

adc.sendCommand(SDATAC): Send SDATAC to stop reading contionus data, so we can send other command

adc.sendCommand(RDATAC): Send SDATAC to read contionusly data

* adc.readRegister(reg): Returns value of the register "reg", 

Example: 

adc.readRegister(MUX): Read the current content of the MUX register, be aware if the conversion cycle is not complete this will probably read garbage. 


## ADC Calibration 
Refer to the datasheet and advanced functions
 

## Dependencies

This code uses Arduino SPI library and some direct port manipulation

## Credits

### Aknowledgement 


* [adienakhmad](https://github.com/adienakhmad), original library
* [chepo92](https://github.com/chepo92)
  
### Maintainers
* [chepo92](https://github.com/chepo92)
  

## Licence, Attributions and Contributions
Licence: MIT

Contributions are welcomed


