#include <ezButton.h>

//limit switch Min GND & pin15 (Min Pedal Travel)
//limit switch Max GND & pin14 (Max Pedal Travel)

ezButton limitSwitchMin(5);  // create ezButton object that attach to pin 2;
ezButton limitSwitchMax(4);  // create ezButton object that attach to pin 3;

//Stepper Motor Information
#include "FastAccelStepper.h"

// Stepper Wiring
#define dirPinStepper 8   //This can be any output capable port pin.
#define stepPinStepper 9  // step pin must be pin 9, 10 or 11

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;


//ADS1256 Setup
#include <ADS1256.h>
#include <SPI.h>

/*Pinouts
  5V   - 5V
  GND  - GND
  SCLK - pin 15 (SCK)
  DIN  - pin 16 (MOSI)
  DOUT - pin 14 (MISO)
  DRDY - pin 6
  CS   - pin 17
  POWN - pin 7
  */

//ADS1256 Variables
float clockMHZ = 7.68;  // crystal frequency used on ADS1256
float vRef = 4.8;       // voltage reference
float sensor1;
float conversion = (-300 / (vRef * 0.002)) * 2.2;  //conversion factor || conversion = (Load Cell Max)/(vRef(V) * Load Cell Sensitivity(V/V)) || 1Kg = 2.2 lb
float offset;                                      //offset value

// Initialize ADS1256 object
ADS1256 adc(clockMHZ, vRef, false);  // RESETPIN is permanently tied to 3.3v


//Moving Average
#include <movingAvgFloat.h>

movingAvgFloat Force_Current_MA(150);  //Sets the number of data points to use when calculating the moving average

//Variable for calculation pedal movement & Stepper Parameters
float Force_Min = 3;   //Min Force in lb to activate Movement
float Force_Max = 20;  //Max Force in lb = Max Travel Position
float Force_Current = 0;

float Position_Min = 0;      //Pedal starting Postion. In the future this could be found using a switch
float Position_Max = 90;     //Pedal maxium distance to travel
float Position_Current = 0;  //Current position of pedal
float Position_Next = 0;     //Future pedal postion based on current load cell data

float steps_per_rev = 1000;
float rpm = 4000;
float speed = (rpm / 60 * steps_per_rev);  //needs to be in us per step || 1 sec = 1000000 us
float accel = 1000000;
int Position_Deadzone = steps_per_rev * 0.01;  //Number of steps required before the pedal will move. Added in to prevent oscillation caused by varaying load cell readings

//Throttle Control
#include <Joystick.h>

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   0, 0,                 // Button Count, Hat Switch Count
                   false, false, false,  // X and Y, but no Z Axis
                   false, false, false,  // No Rx, Ry, or Rz
                   false, false,         // No rudder or throttle
                   true, false, false);  // No accelerator, brake, or steering

void setup() {
  Serial.begin(9600);

  ADS1256_Setup();

  Force_Current_MA.begin();  //Start force moving average

  //FastAccelStepper setup
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  if (stepper) {
    stepper->setDirectionPin(dirPinStepper);
    //stepper->setEnablePin(enablePinStepper);
    stepper->setAutoEnable(true);

    //Stepper Parameters
    stepper->setSpeedInHz(speed);     // steps/s
    stepper->setAcceleration(accel);  // 100 steps/s²
  }

  // Setting Min and Max Travel Position
  //***INSERT positionFinder loop here*** This will move the stepper to its Max and Min position determined by the limit switches
  positionFinder();

  //Throttle Control Setup
  Joystick.setAcceleratorRange(Position_Min, Position_Max);
  delay(100);
  Joystick.begin();
}

void loop() {
  //limitSwitchMax.loop();  // MUST call the loop() function first
  //Serial.println("Switch Ready");

  //while(limitSwitchMax.getState() == HIGH){

  ADS1256_Read();

  if (Force_Current > Force_Max) {  //If current force is over the max force it will just read the max force
    Force_Current = Force_Max;
  }

  if (Force_Current < Force_Min) {  //If current force is below the min force it will just read 0
    Force_Current = Force_Min;
  }

    Position_Next = ((Position_Max - Position_Min) / (Force_Max - Force_Min)) * (Force_Current - Force_Min) + Position_Min;  //Calculates new position using linear function

    if (Position_Next > Position_Max) {  //If current force is over the max force it will just read the max force
    Position_Next = Position_Max;
  }

  if (Position_Next < Position_Min) {  //If current force is below the min force it will just read 0
    Position_Next = Position_Min;
  }

  /*Serial.print(Force_Current, 3);
  Serial.println(" lbs || ");
  /*Serial.print(Position_Next - Position_Current);
  Serial.print(" Travel Distance ");
  Serial.print(Position_Next);
  Serial.print(" senosr lbs ");
  Serial.println(sensor1,4);*/

  stepper->moveTo(Position_Next);

  Position_Current = Position_Next;  //Sets the new current position after the stepper motors moves !!!This has to be after the stepper have moved
  
  Joystick.setAccelerator(Position_Next);
}
