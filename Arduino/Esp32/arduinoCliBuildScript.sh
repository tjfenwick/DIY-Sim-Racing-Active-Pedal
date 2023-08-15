# shell script for automatic built of the ESP32 binaries
# for Arduino-cli help, see https://arduino.github.io/arduino-cli/0.33/commands/arduino-cli_compile/


arduino-cli compile --fqbn esp32:esp32:fm-devkit --output-dir Arduino/Esp32/bin Arduino/Esp32/Main/.
