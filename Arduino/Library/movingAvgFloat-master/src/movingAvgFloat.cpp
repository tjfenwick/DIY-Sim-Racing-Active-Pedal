// Arduino Moving Average Library
// https://github.com/ThingEngineer/movingAvgFloat
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html

#include <movingAvgFloat.h>

// initialize - allocate the interval array
void movingAvgFloat::begin()
{
    m_readings = new float[m_interval];
}

// add a new reading and return the new moving average
float movingAvgFloat::reading(float newReading)
{
    // add each new data point to the sum until the m_readings array is filled
    if (m_nbrReadings < m_interval)
    {
        (float)++m_nbrReadings;
        m_sum = m_sum + newReading;
    }
    // once the array is filled, subtract the oldest data point and add the new one
    else
    {
        m_sum = (float)m_sum - m_readings[m_next] + newReading;
    }

    m_readings[m_next] = newReading;
    if (++m_next >= m_interval) m_next = 0;
    return (float)(m_sum / m_nbrReadings);
}

// just return the current moving average
float movingAvgFloat::getAvg()
{
    return (float)(m_sum / m_nbrReadings);
}

// start the moving average over again
void movingAvgFloat::reset()
{
    m_nbrReadings = 0.0;
    m_sum = 0.0;
    m_next = 0;
}
