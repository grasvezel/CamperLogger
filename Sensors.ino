void readTemperatureSensors() {
  addLog(LOG_LEVEL_INFO, "TEMP : Reading temperature sensors");
  DallasTemperature sensors(&oneWire);
  sensors.requestTemperatures(); // Send the command to get temperatures
  for (int sensorNr = 0; sensorNr < 10; sensorNr++) {
    float temperature = sensors.getTempCByIndex(sensorNr);
    readings.temp[sensorNr] = temperature;
  }
  return;
}

void readWaterTankLevelSensor() {
  int loops = 10;
  int currenttanklevel = 0;

  for (int i = 0; i < loops; i++) {
    currenttanklevel += analogRead(WATER_LEVEL_SENSOR_PIN);
  }
  currenttanklevel = currenttanklevel / loops;

  // tank levels
  // 724 11   -55 mm    235   100%
  // 643 10   -70 mm    220    94%
  // 565  9   -85 mm    205    87%
  // 480  8   -100 mm   180    77%
  // 442  7   -125 mm   165    70%
  // 395  6   -140 mm   150    64%
  // 351  5   -165 mm   125    53%
  // 306  4   -185 mm   105    45%
  // 238  3   -205 mm    85    36%
  // 167  2   -225 mm    65    28%
  // 20   1   -265 mm    25    11%
  // 0    0   -290 mm     0     0%

  readings.Water_level = 0;
  if (currenttanklevel > 10) {
    readings.Water_level = 11;
  }
  if (currenttanklevel > 94) {
    readings.Water_level = 28;
  }
  if (currenttanklevel > 202) {
    readings.Water_level = 36;
  }
  if (currenttanklevel > 272) {
    readings.Water_level = 45;
  }
  if (currenttanklevel > 329) {
    readings.Water_level = 53;
  }
  if (currenttanklevel > 373) {
    readings.Water_level = 64;
  }
  if (currenttanklevel > 419) {
    readings.Water_level = 70;
  }
  if (currenttanklevel > 461) {
    readings.Water_level = 77;
  }
  if (currenttanklevel > 523) {
    readings.Water_level = 87;
  }
  if (currenttanklevel > 604) {
    readings.Water_level = 94;
  }
  if (currenttanklevel > 684) {
    readings.Water_level = 100;
  }
  return;
}

void readGasTankLevelSensor() {
  int loops = 10;
  int currenttanklevel = 0;

  for (int i = 0; i < loops; i++) {
    currenttanklevel += analogRead(GAS_LEVEL_SENSOR_PIN);
  }
  float gas_level = currenttanklevel / loops / 15.75; // max reading (80%) is about 1260.
  readings.Gas_level = (int) gas_level;
}
