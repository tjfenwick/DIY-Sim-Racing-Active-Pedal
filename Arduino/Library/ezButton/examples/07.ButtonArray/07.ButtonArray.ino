/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-button-library
 *
 * This example shows how to use array of button.
 */

#include <ezButton.h>

const int BUTTON_NUM = 5;

const int BUTTON_1_PIN = 2;
const int BUTTON_2_PIN = 3;
const int BUTTON_3_PIN = 4;
const int BUTTON_4_PIN = 5;
const int BUTTON_5_PIN = 6;

ezButton buttonArray[] = {
  ezButton(BUTTON_1_PIN),
  ezButton(BUTTON_2_PIN),
  ezButton(BUTTON_3_PIN),
  ezButton(BUTTON_4_PIN),
  ezButton(BUTTON_5_PIN)
};

void setup() {
  Serial.begin(9600);

  for (byte i = 0; i < BUTTON_NUM; i++) {
    buttonArray[i].setDebounceTime(50); // set debounce time to 50 milliseconds
  }
}

void loop() {
  for (byte i = 0; i < BUTTON_NUM; i++)
    buttonArray[i].loop(); // MUST call the loop() function first

  for (byte i = 0; i < BUTTON_NUM; i++) {
    if (buttonArray[i].isPressed()) {
      Serial.print("The button ");
      Serial.print(i + 1);
      Serial.println(" is pressed");
    }

    if (buttonArray[i].isReleased()) {
      Serial.print("The button ");
      Serial.print(i + 1);
      Serial.println(" is released");
    }
  }
}