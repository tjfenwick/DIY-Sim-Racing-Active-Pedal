/***************************************************
Copyright (c) 2019 Luis Llamas
(www.luisllamas.es)
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
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
