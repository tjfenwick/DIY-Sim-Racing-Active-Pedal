
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
The embedded code of the DIY active pedal runs on an ESP32 microcontroller. Some PCB design, which hold the ESP32 breakoutboard, the ADC breakoutboard and connectors are uploaded to the [Wiring](Wiring) directory. They were used to proof the concept. 
The regular ESP32 is currently recommended over the ESP32 S2, as it has a FPU and it performs better with the stepper library. ESP32 S2 is losing steps currently.

ESP32 PCB            |  ESP32 S2 mini PCB
:-------------------------:|:-------------------------:
![](Wiring/Esp32/Esp32_PCB_0.png)  |  ![](Wiring/Esp32_s2_mini/Esp32_S2_mini_PCB_0.png)

A more sophisticated custom PCB is currently under development and tested. Please refer to the Discord for up-to-date designs.

## Mechanics
There are multiple variations of the DIY active pedal build by users, which can be found at our [discord](https://discord.gg/j9K5vUuT) channel or [here](#other-designs).

## BOM
The [BOM](BOM.md) refers to the pedal design which I have choosen, see below:
![image](Images/Build_1.jpg).
I've designed it mostly from of the shelf norm metal parts, thus it is easy and cheap to reproduce. However, there are multiple ways to build the pedal. I suggest that you only use this BOM as a reference and build your DIY active pedals however you like them. Go to our discord server and search for images to get inspirations.

The first pedal costs me about 300€. Another add-on pedal costed me about 240€, since I bough some of the parts in packs for the first pedal and thus already had them. 

Currently a nicer PCB deisgn is under development, which has the ESP & ADS integrated. It needs to be testet though. I highly recommend to wait for it, as it will be much more user friendly, e.g. the build process will take less than an hour for an experienced craftsman. 

## Other designs
Here is a list of mechanical designs other incredible DIYers have done. 

| Design           |  Link |
:------------------------- | :-------------------------
|<img src="https://user-images.githubusercontent.com/17485523/231913569-695fcab1-f0bb-4af6-8d90-b1bfaece13bc.png" height="200">  |  [Tjfenwick design](https://github.com/tjfenwick/DIY-Sim-Racing-Active-Pedal)|
|<img src="https://user-images.githubusercontent.com/79850208/261399337-b313371c-9262-416d-a131-44fa269f9557.png" height="200">  |  [Bjoes design](https://github.com/Bjoes/DIY-Active-pedal-mechanical-design)|


  
# Software

## ESP32 code

### Architecture
A Doxygen report of the sources can be found [here](Arduino/html/index.html).

### Built from source
To flash the [code](Arduino/Esp32/Main), e.g. via Ardiono IDE to esp32. 

### flash prebuilt binaries via webflasher
HASN'T BEEN TESTED YET:
The prebuilt binaries for the regular ESP32 can be found [here](Arduino/Esp32/bin). They can be flashed via the ESP [webflasher](https://esp.huhn.me/). 

## SimHub plugin:
The SimHub plugin was designed to communicate with the esp32 to (a) modify the pedal configuration, e.g. the force vs. travel parameterization and (b) to trigger effects such as ABS oscillations.  

![image](SimHubPlugin/Images/SimHubPluginOverview.png)

To install the plugin, copy the plugin [binaries](SimHubPlugin/bin) content to your SimHub directory, e.g. C:/Program Files (x86)/SimHub



# Misc
## Pedal kinematics calculation
To get a better understanding of the motion and forces, a [python](Validation/PedalKinematics/main.py) script for simulation of the pedal angle, the pedal angular velocity and maximum pedal force has been written. Feel free to tune the pedal geometry as needed. The simulation result for my pedal geometry looks as follows:

<img src="Validation/PedalKinematics/pedalKinematics.png" width="300">



# Todo
- [ ] Add Doxygen + Graphviz to the project to automatically generate documentation, architecureal design overview, etc.
- [ ] Add automatic system identification of pedal response
- [ ] Add model-predictive-controll to the ESP code for improved pedal response
- [ ] Add build manual
- [ ] Create video describing the build progress and the features
- [ ] https://github.com/gin66/FastAccelStepper/issues/194
- [ ] https://github.com/br3ttb/Arduino-PID-Library/issues/138
- [ ] GUI design improvements for the SimHub plugin
- [ ] SimHub plugin: Bugfix for COM port selection wrong, when switching between pedals
- [ ] Send SimHub data via wifi to ESP
- [ ] Automatically generate the bin files and refer to the ESP32 [webflasher](https://esp.huhn.me/)
