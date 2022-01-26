void writeGpsDataToInfluxDb() {
  // Clear fields for reusing the point. Tags will remain untouched
  sensor.clearFields();

  // Store measured value into point
  // Report RSSI of currently connected network
  sensor.addField("sat", readings.GPS_sat);

  sensor.addField("lat", readings.GPS_lat);
  sensor.addField("lon", readings.GPS_lon);

  sensor.addField("alt", readings.GPS_alt);

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}
