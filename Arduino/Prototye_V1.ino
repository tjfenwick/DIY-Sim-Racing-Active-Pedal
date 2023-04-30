#include <ezButton.h>

 //limit switch 2 5v & pin1 (Min Pedal Travel)
 //limit switch 3 5v & pin2 (Max Pedal Travel)

ezButton limitSwitchMin(2);  // create ezButton object that attach to pin 2;
ezButton limitSwitchMax(3);  // create ezButton object that attach to pin 3;

//Stepper Motor Information
#include "FastAccelStepper.h"

    // Stepper Wiring
    #define dirPinStepper    7 
    #define stepPinStepper   9  // step pin must be pin 9

//no clue what this does
FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;



/*ADS1256 Pinouts
5V - 5V
GND - GND
SCLK - pin 13 (SCK)
DIN - pin 11 (MOSI)
DOUT - pin 12 (MISO)
DRDY - pin 6
CS - pin 10
POWN - 5V
*/

#include <ADS1256.h>
#include <SPI.h>
#include <movingAvgFloat.h>


// Configuration of Kalman filter
// assume constant rate of change 
// observed states:
// x = [force, d force / dt]
// state transition matrix
// x_k+1 = [1, delta_t; 0, 1] * x_k
#include <Kalman.h>
using namespace BLA;

#define LOADCELL_STD_MIN 0.001f
#define LOADCELL_VARIANCE_MIN LOADCELL_STD_MIN*LOADCELL_STD_MIN

// Dimensions of the matrices
#define Nstate 2 // length of the state vector
#define Nobs 1   // length of the measurement vector

// measurement std (to be characterized from your sensors)
#define n1 0.002 // noise on the 1st measurement component // standard deviation of measurement noise. Rule of thumb: assume Gaussian distribution. 99.9% is 3*sigma interval --> (max-min) / 3 = stddev
#define n2 0.1 // noise on the 2nd measurement component 


KALMAN<Nstate,Nobs> K; // your Kalman filter
BLA::Matrix<Nobs, 1> obs; // observation vector





float clockMHZ = 7.68; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference
// Initialize ADS1256 object
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v


float sensor1;
movingAvgFloat movingAvgFilter(40);      //Sets the number of data points to use when calculating the moving average 

float conversion = 4000.0f; //conversion factor
float loadcellOffset = 0.0f;     //offset value
float varEstimate = 0.0f; // estimated loadcell variance
float stdEstimate = 0.0f;

//Variable for calculation pedal movement & Stepper Parameters
float Force_Min = 0.1;    //Min Force in lb to activate Movement
float Force_Max = 10.;     //Max Force in lb = Max Travel Position
float Force_Current_MA = 0.0f; // current force smoothened by MA filter
float Force_Current_EXP = 0.0f; // current force smoothened by exp filter
float alpha = 0.15; // exp filter coefficient
float Force_Current_KF = 0.0f; // current force smoothened by KF filter

long  Position_Min = 0;       //Pedal starting Postion. In the future this could be found using a switch
long  Position_Max = 90;     //Pedal maxium distance to travel
long  Position_Current = 0;   //Current position of pedal
long  Position_Next = 0;      //Future pedal postion based on current load cell data
long  Position_Deadzone = 5;  //Number of steps required before the pedal will move. Added in to prevent oscillation caused by varaying load cell readings

float steps_per_rev = 1600;
float rpm = 2500;
float speed = (rpm/60*steps_per_rev);   //needs to be in us per step || 1 sec = 1000000 us
float accel = 1000000;

long previousTime = 0;


float springStiffnesss = 1;
float springStiffnesssInv = 1;

long prevPosition = 0;
long currentPosition = 0;

void setup()
{
  Serial.begin(2000000);


  movingAvgFilter.begin();     //Start force moving average
  
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
  adc.begin(ADS1256_DRATE_500SPS,ADS1256_GAIN_64,false); 

  Serial.println("ADC Started");
  
   /// Set MUX Register to AINO and AIN1 so it start doing the ADC conversion 
  adc.setChannel(0,1); // switch back to MUX AIN0 and AIN1

  adc.waitDRDY(); // wait for DRDY to go low before changing multiplexer register
    adc.setChannel(0,1);   // Set the MUX for differential between ch2 and 3 
    sensor1 = (adc.readCurrentChannel()*conversion); // DOUT arriving here are from MUX AIN0 and AIN1

  // Due to construction and gravity, the loadcell measures an initial voltage difference.
  // To compensate this difference, the difference is estimated by moving average filter.
  float ival = 0;
  long samples = 1000;
  
  for (long i = 0; i < samples; i++){
    adc.setChannel(0,1);   // Set the MUX for differential between ch2 and 3 
    sensor1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and 
    ival = sensor1 / samples;
    Serial.println(sensor1,10);
    loadcellOffset += ival;
  }

  Serial.print("Offset ");
  Serial.println(loadcellOffset,10);
  delay(1000);

  // automatically identify sensor noise for KF parameterization
  float varNormalizer = 1. / (samples - 1);
  varEstimate = 0.0f;
  for (long i = 0; i < samples; i++){
    adc.setChannel(0,1);   // Set the MUX for differential between ch2 and 3 
    sensor1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and 
    ival = (sensor1 - loadcellOffset);
    ival *= ival;
    varEstimate += ival * varNormalizer;
  }
  varEstimate *= conversion * conversion; // respect linear scaling y = mu * x --> var(y) = mu^2 * var(x)

  // make sure estimate is nonzero
  if (varEstimate < LOADCELL_VARIANCE_MIN){varEstimate = LOADCELL_VARIANCE_MIN; }
  stdEstimate = sqrt(varEstimate);


  Serial.print("stdEstimate:");
  Serial.println(stdEstimate, 6);
  
  




    
  //FastAccelStepper setup
  engine.init();
  stepper = engine.stepperConnectToPin(stepPinStepper);
  if (stepper) {
    stepper->setDirectionPin(dirPinStepper);
    //stepper->setEnablePin(enablePinStepper);
    stepper->setAutoEnable(true);
    
  //Stepper Parameters
    stepper->setSpeedInHz(speed);   // steps/s
    stepper->setAcceleration(accel);  // 100 steps/s²

  }

  // Setting Min and Max Travel Position
  
  limitSwitchMin.setDebounceTime(50); // set debounce time to 50 milliseconds
  limitSwitchMax.setDebounceTime(50); // set debounce time to 50 milliseconds

 limitSwitchMin.loop();  // MUST call the loop() function first
 limitSwitchMax.loop();  // MUST call the loop() function first

  //long stateMin = limitSwitchMin.getState();
  //long stateMax = limitSwitchMax.getState();
  

  long set = 0;                                                 //Setting Max Limit Switch Position
  while(limitSwitchMax.getState() == HIGH){
  stepper->setSpeedInHz(speed);   // steps/s      
  stepper->moveTo(set);       //Move motor 10 steps back
    
    while (stepper->getCurrentPosition() != stepper->targetPos()){}
    

    set = set + 100;
    //Serial.println(set);
        
    limitSwitchMax.loop();  // MUST call the loop() function first

 
    if(limitSwitchMax.getState() == LOW){
        Serial.println("The limit switch: Max On");
        Serial.print("Max Position is "); 
        Serial.println(stepper->getCurrentPosition()-1000);

      Position_Max = (stepper->getCurrentPosition()-1000);
    } 
  }    

  while(limitSwitchMin.getState() == HIGH){                    //Setting Min Limit Switch Position
    stepper->setSpeedInHz(speed);   // steps/s
    stepper->moveTo(set);       //Move motor 10 steps back

    while (stepper->getCurrentPosition() != stepper->targetPos()){}

    set = set - 100;
    //Serial.println(set);

    limitSwitchMin.loop();  // MUST call the loop() function first

    if(limitSwitchMin.getState() == LOW){
      Serial.println("The limit switch: Min On");
      Serial.print("Min Position is ");
      Serial.println(stepper->getCurrentPosition()+6000);
      
      Position_Min = (stepper->getCurrentPosition()+6000);
    }
  }   

stepper->moveTo(Position_Min);       
      while (stepper->getCurrentPosition() != stepper->targetPos()){
      delay(10);
      }

stepper->setSpeedInHz(speed);

  // compute pedal stiffness parameters
  springStiffnesss = (Force_Max-Force_Min) / (float)(Position_Max-Position_Min);
  springStiffnesssInv = 1.0 / springStiffnesss;

  // obtain current stepper position
  prevPosition = stepper->getCurrentPosition();




  // example of evolution matrix. Size is <Nstate,Nstate>
  K.F = {1.0, 0.0,
         0.0, 1.0};
         
  // example of measurement matrix. Size is <Nobs,Nstate>
  K.H = {1.0, 0.0};

  // example of measurement covariance matrix. Size is <Nobs,Nobs>
  //K.R = {n1*n1};
  K.R = {varEstimate};
  

  // example of model covariance matrix. Size is <Nstate,Nstate>
  K.Q = {1.0f,   0.0,
           0.0, 1.0f};



}



void loop()
{ 


  // obtain time
  long currentTime = millis();
  long elapsedTime = currentTime - previousTime;
  previousTime = currentTime;

  // obtain loadcell reading measurement
  adc.waitDRDY(); // wait for DRDY to go low before changing multiplexer register
  adc.setChannel(0,1);   // Set the MUX for differential between ch2 and 3 
  sensor1 = adc.readCurrentChannel(); // DOUT arriving here are from MUX AIN0 and AIN1

  // convert loadcell reading to force
  sensor1 = ( sensor1 - loadcellOffset ) * conversion;
  sensor1 = -1.0f * sensor1; // flip sign to make pressure positive

  // low pass filter the force readings
  Force_Current_MA = movingAvgFilter.reading(sensor1);
  Force_Current_EXP = Force_Current_EXP * (1.0f - alpha) + sensor1 * alpha;


  // Kalman filter  
  // update state transition and system covariance matrices
  float delta_t = (float)elapsedTime / 1000.0f;
  K.F = {1.0,  delta_t,
         0.0,  1.0};

  float delta_t_pow2 = delta_t * delta_t;
  float delta_t_pow3 = delta_t_pow2 * delta_t;
  float delta_t_pow4 = delta_t_pow3 * delta_t;
  K.Q = {0.25f * delta_t_pow4,   0.5f * delta_t_pow3,
        0.5f * delta_t_pow3, delta_t_pow2};

  // APPLY KALMAN FILTER
  obs(0) = sensor1;
  K.update(obs);
  Force_Current_KF = K.x(0,0);

  //K.x
  //Serial << obs << ' ' << K.x << '\n';




  // get current stepper position
  currentPosition = stepper->getCurrentPosition();

  // estimate pedal force change in next cycle
//#define COMPENSATE_FORCE_CHANGE
#ifdef COMPENSATE_FORCE_CHANGE
  float expectedForceIncrease = 1.0 * springStiffnesss * (float)(currentPosition - prevPosition);
  Force_Current -= expectedForceIncrease;
#endif
  // estimate target pedal position
  //Position_Next = springStiffnesssInv * (Force_Current_MA-Force_Min) + Position_Min ;        //Calculates new position using linear function
  //Position_Next = springStiffnesssInv * (Force_Current_EXP-Force_Min) + Position_Min ;        //Calculates new position using linear function
  Position_Next = springStiffnesssInv * (Force_Current_KF-Force_Min) + Position_Min ;        //Calculates new position using linear function
  
  // clip target pedal position
  if (Position_Next > Position_Max)  {       //If current force is over the max force it will just read the max force
    Position_Next = Position_Max;
  }
  if (Position_Next < Position_Min)  {       //If current force is below the min force it will just read 0
    Position_Next = Position_Min;
  }

  float targetPosRel = ( (float)(Position_Next-Position_Min)) / ( (float)(Position_Max - Position_Min) );

// write debug info to serial monitor
#define DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
  Serial.print("elapsedTime:");
  Serial.print(elapsedTime);
  Serial.print(",");
  Serial.print("instantaneousForceMeasured:");
  Serial.print(sensor1,6);
  Serial.print(",");
  Serial.print("Force_Current_MA:");
  Serial.print(Force_Current_MA, 6);
  Serial.print(",");
  Serial.print("Force_Current_EXP:");
  Serial.print(Force_Current_EXP, 6);
  Serial.print(",");

  Serial.print("Kalman_x:");
  Serial.print(Force_Current_KF, 6);
  Serial.print(",");

  Serial.print("Kalman_x_d:");
  Serial.print(K.x(1,0), 6);
  Serial.print(",");

  Serial.print("TravelDistance:");
  Serial.print(Position_Next - Position_Current);
  Serial.print(",");
  Serial.print("targetPosRel:");
  Serial.print(targetPosRel,5);
  Serial.println(" ");
#endif

  if(abs(Position_Next-Position_Current)>Position_Deadzone){       //Checks to see if the new position move is greater than the deadzone limit set to prevent oscillation
    stepper->moveTo(Position_Next);
  }


  // 
  prevPosition = currentPosition;

 }
