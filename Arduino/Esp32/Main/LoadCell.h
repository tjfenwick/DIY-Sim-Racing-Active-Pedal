#pragma once

#include <stdint.h>

class LoadCell_ADS1256 {
private:
  float _zeroPoint;
  float _varianceEstimate;

public:
  LoadCell_ADS1256(uint8_t channel0=0, uint8_t channel1=1);
  float getReadingKg() const;
  
public:
  void setZeroPoint();
  void estimateVariance();

public:
  float getVarianceEstimate() const { return _varianceEstimate; }
};
