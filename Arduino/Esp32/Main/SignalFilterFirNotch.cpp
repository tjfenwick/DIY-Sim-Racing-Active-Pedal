#include "SignalFilterFirNotch.h"



  // Very simple Finite Impulse Response notch filter
  
  auto filter2 = simpleNotchFIR(2 * 2. * 15. / 10000.); // second harmonic




  // Sampling frequency
  const double f_s_butter = 10000; // Hz
  // Cut-off frequency (-3 dB)
  const double f_c_butter = 15; // Hz
  // Normalized cut-off frequency
  const double f_n_butter = 2 * f_c_butter / f_s_butter;
  auto filter = butter<6>(f_n_butter);

  auto filter1 = simpleNotchFIR(2.0 * 1500. / 10000.);     // fundamental


FirNotchFilter::FirNotchFilter(float notchFrequency)
{
  f_s = 100000.; // Hz
  // Notch frequency (-∞ dB)
  f_c = 15;
  // Normalized notch frequency
  f_n = 2 * f_c / f_s;

  filter1 = simpleNotchFIR(f_n);     // fundamental
  //filter2 = simpleNotchFIR(2 * f_n); // second harmonic

}

float FirNotchFilter::filterValue(float value) {
  //return filter2(filter1(value));
  return filter1(value);
  //return filter(value);
}

