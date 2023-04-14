// Arduino Floating Point Moving Average Library
// https://github.com/ThingEngineer/movingAvgFloat
// Copyright (C) 2018 by Jack Christensen & Josh Campbell
// Licensed under GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html

#ifndef MOVINGAVGFLOAT_H_INCLUDED
#define MOVINGAVGFLOAT_H_INCLUDED

class movingAvgFloat
{
    public:
        movingAvgFloat(int interval)
            : m_interval(interval), m_nbrReadings(0), m_sum(0), m_next(0) {}
        void begin();
        float reading(float newReading);
        float getAvg();
        void reset();

    private:
        int m_interval;       // number of data points for the moving average
        float m_nbrReadings;  // number of readings
        float m_sum;          // sum of the m_readings array
        int m_next;           // index to the next reading
        float *m_readings;    // pointer to the dynamically allocated interval array
};
#endif
