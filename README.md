
# DIY-Sim-Racing-Active-Pedal
This DIY active pedal design is based on the original design from @tjfenwick.



# Disclaimer
This product is essentialy a robot. If not interacted with care, it may cause harm.
I'm not responsible for any harm caused by this design suggestion. Use responsible and at your own risk.  


# Discord
For better communication, a [Discord](https://discord.gg/j8QhD5hCv7) server has been created. 
Feel free to join, research before you build and ask questions.


# Hardware
## ESP32
The embedded code of the DIY active pedal runs on an ESP32 microcontroller. Some beakoutboards are uploaded to the [Wiring](Wiring) directory. 
The regular ESP32 is currently recommended over the ESP32 S2, as it has a FPU and it performs better with the stepper library. ESP32 S2 currently is losing steps. 

A more sophisticated custom PCB is currently under development and is beeing tested. Please refer to the Discord for up-to-date designs.

## Mechanics
There are multiple variations of the DIY active pedal build by users, which can be found at our [discord](https://discord.gg/j9K5vUuT) channel.   

## BOM
The [BOM](BOM.md) refers to the pedal design which I have choosen, see below:
![image](Images/Build_1.jpg).
I've designed it mostly from of the shelf norm metal parts, thus it is easy and cheap to reproduce. However, there are multiple ways to build the pedal. I suggest that you only use this BOM as a reference and build your DIY active pedals however you like them. Go to our discord server and search for images to get inspirations.

The first pedal costs me about 300€. Another add-on pedal costed me about 240€, since I bough some of the parts in packs for the first pedal and thus already had them. 

Currently a nicer PCB deisgn is under development, which has the ESP & ADS integrated. It needs to be testet though. I highly recommend to wait for it, as it will be much more user friendly, e.g. the build process will take less than an hour for an experienced craftsman. 

  
# Software

## ESP32 code
To flash the [code](Arduino/Esp32/Main), e.g. via Ardiono IDE to esp32. 

## SimHub plugin:
The SimHub plugin was designed to communicate with the esp32 to (a) modify the pedal configuration, e.g. the force vs. travel parameterization and (b) to trigger effects such as ABS oscillations.  

![image](SimHubPlugin/Images/SimHubPluginOverview.png)

To install the plugin, copy the plugin [binaries](SimHubPlugin/bin) content to your SimHub directory, e.g. C:/Program Files (x86)/SimHub
