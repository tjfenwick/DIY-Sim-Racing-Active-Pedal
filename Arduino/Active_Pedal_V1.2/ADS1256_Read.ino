void ADS1256_Read() {

  adc.waitDRDY();
  sensor1 = ((adc.readCurrentChannel()-offset)*conversion);  // DOUT arriving here are from MUX AIN0 and AIN1
  Force_Current = Force_Current_MA.reading(sensor1);
}