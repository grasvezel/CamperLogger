void readTemperatureSensors() {
  addLog(LOG_LEVEL_INFO, "TEMP : Reading temperature sensors");
  DallasTemperature sensors(&oneWire);
  sensors.requestTemperatures(); // Send the command to get temperatures
  for (int sensorNr = 0; sensorNr < 10; sensorNr++) {
    float temperature = sensors.getTempCByIndex(sensorNr);
    //Serial.print("temp nr:");Serial.print(sensorNr);Serial.print(" val:");Serial.println(temperature);    
    readings.temp[sensorNr] = temperature;
  }
  return;
}
