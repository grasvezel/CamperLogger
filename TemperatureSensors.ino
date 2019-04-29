String readTemperatureSensors() {
  DallasTemperature sensors(&oneWire);
  String request = "";
  sensors.requestTemperatures(); // Send the command to get temperatures
  for(int sensorNr=0;sensorNr<10;sensorNr++) {
    float temperature = sensors.getTempCByIndex(sensorNr);
    if(temperature != -127.00) {
      request += "&T" + String(sensorNr) + "=" + String(temperature);
    }
  }
  return(request);
}
