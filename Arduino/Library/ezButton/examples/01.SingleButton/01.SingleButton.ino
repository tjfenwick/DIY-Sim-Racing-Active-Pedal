/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-button-library
 *
 * This example reads the state of a button without debounce and print it to Serial Monitor.
 */

#include <ezButton.h>

ezButton button(7);  // create ezButton object that attach to pin 7;

void setup() {
  Serial.begin(9600);
}

void loop() {
  button.loop(); // MUST call the loop() function first

  int btnState = button.getState();
  Serial.println(btnState);
}
