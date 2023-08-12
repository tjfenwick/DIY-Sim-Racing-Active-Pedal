

# DIY-Sim-Racing-Active-Pedal
This is my DIY Simucube Active Pedal prototype, an affordable alternative to the official Simucube active pedals that cost over $2,000 per pedal. The pedal uses a load cell to measure the force applied to the pedal, a NEMA 23 Integrated Easy Servo Motor to drive the lead screw, and an Arduino Uno (soon to be Teensy 4.0) to control everything. I used Fusion 360 to create CAD models for all the 3d printed parts. While there are still some issues to address, such as shaky ADC readings and a lack of computing power on the Arduino, I'm proud of what I've achieved so far and hope to inspire others to take on similar DIY projects. Check out the video of the pedal in action, and feel free to contribute to the project or share your own DIY ideas!

![image](https://user-images.githubusercontent.com/17485523/231913569-695fcab1-f0bb-4af6-8d90-b1bfaece13bc.png)

https://www.youtube.com/watch?v=hGqJAhKMDJU

Current Build of Materials ~$250 **Some of these parts will be switched out as I only picked them cause I had them laying around
- 300kg Load Cell $36.69 
- NEMA 23 Integrated Easy Servo Motor 130w ( iSV57T-130 ) $93.93 (Could probably get away with the smaller 90 watt version and cut down on the overall length of the pedal)
- ADS1256 $17.99
- 8x 608-2RS Ball Bearing $8.99
- Tr8x8 Lead Screw with T8 Brass Nut $11.99 (upgrading to a ball screw in the future)
- 8mm to 8mm Shaft Coupling $9.99
- Arduino Uno $16.99 (upgrading to a Teensy 4.0 in the future)
- 2x Micro Limit Switch $5.99
- DROK 48V Power Supply, AC 110V/220V to DC 0-48V 10A 480W $40.99


# Disclaimer
This product is essentialy a robot. If not interacted with care, it may cause harm.
I'm not responsible for any harm caused by this design suggestion. Use responsible and at your own risk.  


# Discord
For better communication, a [Discord](https://discord.gg/j8QhD5hCv7) server has been created. 
Feel free to join, research before you build and ask questions.


# Hardware
## ESP32
The embedded code of the DIY active pedal runs on an ESP32 microcontroller. 

## Mechanics
There are multiple variations of the DIY active pedal build by users, which can be found at our [discord](https://discord.gg/j9K5vUuT) channel.   

## BOM
The [BOM](BOM.md) refers to the pedal design which I have choosen. There are multiple ways to build the pedal. I suggest that you only use this BOM as a reference and build your DIY active pedals however you like them. Go to our discord server and search for images to get inspirations.

The first pedal costs me about 300€. Another add-on pedal costed me about 240€, since I bough some of the parts in packs for the first pedal and thus already had them. 

Currently a nicer PCB deisgn is under development, which has the ESP & ADS integrated. It needs to be testet though. I highly recommend to wait for it, as it will be much more user friendly, e.g. the build process will take less than an hour for an experienced craftsman. 

  
# Software

## ESP32 code
To flash the [code](Arduino/Esp32/Main), e.g. via Ardiono IDE to esp32. 

## SimHub plugin:
The SimHub plugin was designed to communicate with the esp32 to (a) modify the pedal configuration, e.g. the force vs. travel parameterization and (b) to trigger effects such as ABS oscillations.  

![image](SimHubPlugin/Images/SimHubPluginOverview.png)

To install the plugin, copy the plugin [binaries](SimHubPlugin/bin) content to your SimHub directory, e.g. C:/Program Files (x86)/SimHub
