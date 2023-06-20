# Arduino Interpolation Library
Arduino library that provides interpolation methods step, linear, smooth, catmull spline and constrained spline.

All methods recieves X-Values and Y-Values, the size of this arrays and the X point to interpolate, and return the estimated Y at the point X.

## Modes
### Step
Simple step interpolation. Estimated value is Yn-1 or Yn. The relative 'change point' int he interval (or threshold) is an optional parameter. 0.0 means change at the start of the interval, 1.0 at the and of the interval, and 0.5 in the mid point interval.
![Image](https://github.com/luisllamasbinaburo/Arduino-Interpolation/blob/master/images/arduino-interpolation-step.png)

### Linear
Linear interpolation. Adicional parameter controls the interpolation out of the provided X-Values arrays.
![Image](https://github.com/luisllamasbinaburo/Arduino-Interpolation/blob/master/images/arduino-interpolation-linear.png)

### Smooth
Applies a cubic smooth step between value changes
![Image](https://github.com/luisllamasbinaburo/Arduino-Interpolation/blob/master/images/arduino-interpolation-smooth.png)

### Catmull spline
Typical Catmull spline interpolation
![Image](https://github.com/luisllamasbinaburo/Arduino-Interpolation/blob/master/images/arduino-interpolation-catmull-spline.png)

### Constrained spline
A special kind of spline that doesn't overshoot
![Image](https://github.com/luisllamasbinaburo/Arduino-Interpolation/blob/master/images/arduino-interpolation-constrained-spline.png)


## Example
```c++
#include "InterpolationLib.h"

const int numValues = 10;
double xValues[10] = {   5,  12,  30,  50,  60,  70,  74,  84,  92, 100 };
double yValues[10] = { 150, 200, 200, 200, 180, 100, 100, 150, 220, 320 };

void setup()
{
	while (!Serial) { ; }
	Serial.begin(115200);

	for (float xValue = 0; xValue <= 110; xValue += .25)
	{
	  Serial.print(Interpolation::Step(xValues, yValues, numValues, xValue, 0.0));
		Serial.print(',');
		Serial.print(Interpolation::Step(xValues, yValues, numValues, xValue, 0.5));
		Serial.print(',');
		Serial.print(Interpolation::Step(xValues, yValues, numValues, xValue, 1.0));
		Serial.print(',');
		Serial.print(Interpolation::SmoothStep(xValues, yValues, numValues, xValue));
		Serial.print(',');
		Serial.print(Interpolation::Linear(xValues, yValues, numValues, xValue, false));
		Serial.print(',');
		Serial.print(Interpolation::Linear(xValues, yValues, numValues, xValue, true));
		Serial.print(',');
		Serial.print(Interpolation::CatmullSpline(xValues, yValues, numValues, xValue));
		Serial.print(',');
		Serial.println(Interpolation::ConstrainedSpline(xValues, yValues, numValues, xValue));
	}
}

void loop()
{
}
```

## Auxiliar tools
Aditional utils

### Float map
A simple map function that uses templates, so it works with integer (like normal 'map' function), float, double, or any other comparable type. 
```c++
Interpolation::Map<float>(2.0, 0.0, 10, 100, 200)
```

### Range generator
A simple utility that generates static double arrays with fixed size, and values between 'min' and 'max'. Useful for fast generating 
homogeneously distributed X-values arrays to use as parameter in the interpolations methods. 
```c++
double*  ptr = Range<size>::Generate(min, max);
```



