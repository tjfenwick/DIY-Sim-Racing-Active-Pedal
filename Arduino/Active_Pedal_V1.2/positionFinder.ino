void positionFinder(){
  limitSwitchMin.setDebounceTime(50);  // set debounce time to 50 milliseconds
  limitSwitchMax.setDebounceTime(50);  // set debounce time to 50 milliseconds

  limitSwitchMax.loop();  // MUST call the loop() function first
  limitSwitchMin.loop();  // MUST call the loop() function first

  int set = 0;
  while (limitSwitchMax.getState() == HIGH) {
    stepper->moveTo(set);  //Move motor 10 steps back
    while (stepper->getCurrentPosition() != stepper->targetPos()) {}


    set = set + (0.015 * steps_per_rev);
    Serial.println(set);
    limitSwitchMax.loop();  // MUST call the loop() function first
    limitSwitchMin.loop();  // MUST call the loop() function first
  }

  Serial.println("The limit switch: Max On");
  Serial.print("Max Position is ");
  Serial.println(stepper->getCurrentPosition() - 3000);
  Position_Max = (stepper->getCurrentPosition() - 3000);

  set = 0;
  while (limitSwitchMin.getState() == HIGH) {
    stepper->moveTo(set);  //Move motor 10 steps back
    while (stepper->getCurrentPosition() != stepper->targetPos()) {}

    set = set - (0.015 * steps_per_rev);
    Serial.println(set);
    limitSwitchMax.loop();  // MUST call the loop() function first
    limitSwitchMin.loop();  // MUST call the loop() function first
  }

  Serial.println("The limit switch: Min On");
  Serial.print("Min Position is ");
  Serial.println(stepper->getCurrentPosition() + 2500);
  Position_Min = (stepper->getCurrentPosition() + 2500);

  stepper->moveTo(Position_Min);
  while (stepper->getCurrentPosition() != stepper->targetPos()) {
    delay(10);
  }
}