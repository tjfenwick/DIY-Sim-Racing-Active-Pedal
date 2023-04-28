void ADS1256_Setup() {
  //Seting up ADS1256
  delay(2500);
  Serial.println("Starting ADC");

  // start the ADS1256 with data rate of 15 SPS || Faster speed require more
  // other data rates:
  // ADS1256_DRATE_30000SPS
  // ADS1256_DRATE_15000SPS
  // ADS1256_DRATE_7500SPS
  // ADS1256_DRATE_3750SPS
  // ADS1256_DRATE_2000SPS
  // ADS1256_DRATE_1000SPS
  // ADS1256_DRATE_500SPS
  // ADS1256_DRATE_100SPS
  // ADS1256_DRATE_60SPS
  // ADS1256_DRATE_50SPS
  // ADS1256_DRATE_30SPS
  // ADS1256_DRATE_25SPS
  // ADS1256_DRATE_15SPS
  // ADS1256_DRATE_10SPS
  // ADS1256_DRATE_5SPS
  // ADS1256_DRATE_2_5SPS
  //
  // NOTE : Data Rate vary depending on crystal frequency. Data rates listed below assumes the crystal frequency is 7.68Mhz
  //        for other frequency consult the datasheet.
  //Posible Gains
  //ADS1256_GAIN_1 FULL SCALE INPUT VOLTAGE +- 5V
  //ADS1256_GAIN_2 FULL SCALE INPUT VOLTAGE +- 2.5V
  //ADS1256_GAIN_4 FULL SCALE INPUT VOLTAGE +- 1.25V
  //ADS1256_GAIN_8 FULL SCALE INPUT VOLTAGE +- 0.625V
  //ADS1256_GAIN_16 FULL SCALE INPUT VOLTAGE +- 312.5mV
  //ADS1256_GAIN_32 FULL SCALE INPUT VOLTAGE +- 156.25mV
  //ADS1256_GAIN_64 FULL SCALE INPUT VOLTAGE +- 78.125mV
  adc.begin(ADS1256_DRATE_15000SPS, ADS1256_GAIN_64, false);

  Serial.println("ADC Started");

  // Set MUX Register to AINO and AIN1 so it start doing the ADC conversion
  adc.setChannel(0, 1);  // switch back to MUX AIN0 and AIN1

  adc.waitDRDY();                        // wait for DRDY to go low before changing multiplexer register
  adc.setChannel(0, 1);                  // Set the MUX for differential between ch2 and 3
  sensor1 = (adc.readCurrentChannel());  // DOUT arriving here are from MUX AIN0 and AIN1

  int i;
  float sval = 0;
  float ival = 0;
  int samples = 5000;

  for (i = 0; i < samples; i++) {
    adc.waitDRDY();
    //adc.setChannel(0, 1);                // Set the MUX for differential between ch2 and 3
    sensor1 = (adc.readCurrentChannel());  // DOUT arriving here are from MUX AIN0 and
    ival = sensor1;
    //Serial.println(sensor1,10);
    sval = sval + ival;
  }

  offset = sval / samples;

  Serial.print("Offset ");
  Serial.println(offset, 10);
  delay(1000);
}