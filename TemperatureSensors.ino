void readTemperatureSensors() {
  DallasTemperature sensors(&oneWire);
  sensors.requestTemperatures(); // Send the command to get temperatures
  for(int sensorNr=0;sensorNr<10;sensorNr++) {
    float temperature = sensors.getTempCByIndex(sensorNr);
    readings.temp[sensorNr] = temperature;
  }
  return;
}
