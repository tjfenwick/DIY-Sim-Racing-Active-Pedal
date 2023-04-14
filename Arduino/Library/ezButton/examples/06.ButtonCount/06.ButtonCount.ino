/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-button-library
 *
 * This example reads the number of the pressed count of a button with debounce and print it to Serial Monitor.
 */

#include <ezButton.h>

ezButton button(7);  // create ezButton object that attach to pin 7;

void setup() {
  Serial.begin(9600);
  button.setDebounceTime(50); // set debounce time to 50 milliseconds
  button.setCountMode(COUNT_FALLING);
}

void loop() {
  button.loop(); // MUST call the loop() function first

  unsigned long count = button.getCount();
  Serial.println(count);
}