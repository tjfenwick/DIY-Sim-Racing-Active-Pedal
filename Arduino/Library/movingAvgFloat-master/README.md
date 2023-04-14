# Arduino Floating Point Moving Average Library
https://github.com/ThingEngineer/movingAvg
README file  
Jack Christensen  
Mar 2012

Josh Campbell
Sep 2018

## Change log
-Fork - floating point version

## License
Arduino movingAvg Library Copyright (C) 2018 Jack Christensen GNU GPL v3.0
Arduino movingAvgFloat Library Copyright (C) 2018 Josh Campbell GNU GPL v3.0

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License v3.0 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/gpl.html>

## Description
*movingAvgFloat* is a simple Arduino library for calculating floating point moving averages. It is useful for smoothing formula results, etc. The library operates in the floating point domain. This means that the calculated moving averages are mathematically exact. Both data input to the library and the returned moving averages are of the type: float. Adapted from Jack Christensen's integer version.

The user specifies the interval (number of data points) for the moving average in the constructor. When the `begin()` function is called, an array is dynamically allocated to hold the number of data points in the interval. This array is never deallocated, and the user should call `begin()` only once (for a given `movingAvgFloat` instance) in setup or other initialization code. Dynamic allocation is used strictly with the intent of creating the proper size array for the user's purposes, and not to free up the memory at a later point. It is strongly recommended that `movingAvgFloat` objects remain allocated as long as the code is running. Failure to observe these guidelines can result in heap fragmentation, crashes and other undesired behavior.

## Constructor
### movingAvgFloat(float interval)
##### Description
Defines a `movingAvgFloat` object where the average is calculated using *interval* data points.
##### Syntax
`movingAvgFloat(interval);`
##### Parameters
**interval:** The number of data points to use when calculating the moving average. *(float)*
##### Returns
None.
##### Example
```c++
movingAvgFloat myAvg(10);    // use 10 data points for the moving average
```

## Methods
### begin()
##### Description
Initializes a `movingAvgFloat` object. Call `begin()` once and only once for any given `movingAvgFloat` instance. See comments in the **Description** section above.
##### Syntax
`begin();`
##### Parameters
None.
##### Returns
None.
##### Example
```c++
movingAvgFloat myAvg(10);    // use 10 data points for the moving average
myAvg.begin();
```

### reading(float dataPoint)
##### Description
Adds a new data point to the moving average. Returns the new moving average value. Until the interval array is filled, the average is calculated from those data points already added, i.e. a fewer number of points than defined by the constructor - thanks to Tom H. (Duckie) for this idea!
##### Syntax
`reading(dataPoint);`
##### Parameters
**dataPoint:** The new data point to be added to the moving average. *(float)*
##### Returns
The new moving average value. *(float)*
##### Example
```c++
float formulaResult = (5 / 10);
float formulaResultAvg = myAvg.reading(formulaResult);
```

### getAvg()
##### Description
Returns the current moving average value without adding a new reading.
##### Syntax
`getAvg();`
##### Parameters
None.
##### Returns
The moving average value. *(float)*
##### Example
```c++
float formulaResultAvg = myAvg.getAvg();
```

### reset()
##### Description
Restarts the moving average. Zeros the interval array and associated data.
##### Syntax
`reset();`
##### Parameters
None.
##### Returns
None.
##### Example
```c++
myAvg.reset();
```
