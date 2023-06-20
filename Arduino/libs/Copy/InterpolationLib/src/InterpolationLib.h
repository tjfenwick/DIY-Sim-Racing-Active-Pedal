/***************************************************
Copyright (c) 2019 Luis Llamas
(www.luisllamas.es)
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
 #ifndef _INTERPOLATIONLIB_h
#define _INTERPOLATIONLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


template<size_t n>
struct Range
{
	double list[n];

	Range()
	{
		for (size_t m = 0; m != n; ++m)
		{
			list[m] = m + 1;
		}
	}

	Range(double min, double max)
	{
		for (size_t m = 0; m < n; ++m)
		{
			list[m] = min + (max - min) / (n - 1) * m;
		}
	}

	double& operator[](size_t  index)
	{
		return list[index];
	}

	double* ToArray()
	{
		return list;
	}

	static double* Generate(double min, double max)
	{
		Range<10> range(min, max);
		return range.ToArray();
	}
};



class Interpolation
{
public:
	template <typename T>
	static T Map(T x, T in_min, T in_max, T out_min, T out_max);

	static double Step(double yValues[], int numValues, double pointX, double threshold = 1);
	static double Step(double minX, double maxX, double yValues[], int numValues, double pointX, double threshold = 1);
	static double Step(double xValues[], double yValues[], int numValues, double pointX, double threshold = 1);

	static double Linear(double yValues[], int numValues, double pointX, bool trim = true);
	static double Linear(double minX, double maxX, double yValues[], int numValues, double pointX, bool trim = true);
	static double Linear(double xValues[], double yValues[], int numValues, double pointX, bool trim = true);

	static double SmoothStep(double xValues[], double yValues[], int numValues, double pointX, bool trim = true);
	static double CatmullSpline(double xValues[], double yValues[], int numValues, double pointX, bool trim = true);
	static double ConstrainedSpline(double xValues[], double yValues[], int numValues, double pointX, bool trim = true);

private:
	static double catmullSlope(double x[], double y[], int n, int i);
	static double getFirstDerivate(double x[], double y[], int n, int i);
	static double getLeftSecondDerivate(double x[], double y[], int n, int i);
	static double getRightSecondDerivate(double x[], double y[], int n, int i);

};


// Esto esta aqui porque Arduino la lia con los Templates
template <typename T>
T Interpolation::Map(T x, T in_min, T in_max, T out_min, T out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif