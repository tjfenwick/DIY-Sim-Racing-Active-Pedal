#pragma once


#include <Filters.h>
#include <Filters/Notch.hpp>
#include <Filters/Butterworth.hpp>



class FirNotchFilter {
private:
  double f_s = 1. / (100. / 1000000. ); // Hz
  // Notch frequency (-∞ dB)
  double f_c = 15;//dap_calculationVariables_st.absFrequency; // Hz
  // Normalized notch frequency
  double f_n = 2 * f_c / f_s;

  //auto filter1 = simpleNotchFIR(f_n);     // fundamental
  //auto filter2 = simpleNotchFIR(2 * f_n); // second harmonic

public:
  FirNotchFilter(float notchFrequency);
  float filterValue(float value);
};
