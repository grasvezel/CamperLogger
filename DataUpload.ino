void uploadInfluxGPS() {
      addLog(LOG_LEVEL_INFO, "DATA : upload to influxdb");
  
  if (!Settings.upload_influx) {
      addLog(LOG_LEVEL_INFO, "DATA : influxdb upload disabled");
    return;
  }

  if (GPS_present) {
      addLog(LOG_LEVEL_INFO, "DATA : upload to influxdb");  
      writeGpsDataToInfluxDb();
      /*
    if (Settings.influx_write_geohash) {
      // strings must be quoted
      addLog(LOG_LEVEL_INFO, "DATA : Writing geohash to influxdb");
      influx_post("Geohash", "\"" + String(readings.GPS_geohash) + "\"", "geo");
    }
    if (Settings.influx_write_coords) {
      addLog(LOG_LEVEL_INFO, "DATA : Writing GPS coordinates to influxdb");
      influx_post("lat", String(readings.GPS_lat));
      influx_post("lon", String(readings.GPS_lon));
    }
    if (Settings.influx_write_speed_heading) {
      addLog(LOG_LEVEL_INFO, "DATA : Writing GPS speed & heading to influxdb");
      influx_post("speed", String(readings.GPS_speed));
      influx_post("heading", String(readings.GPS_heading));
    }*/
  }else{
    addLog(LOG_LEVEL_INFO, "DATA : gps data not present");
  }
}

void uploadInfluxReadings() {
  if (!Settings.upload_influx) {
    return;
  }

  addLog(LOG_LEVEL_INFO, "DATA : Writing data to influxdb");

}

void uploadGetData() {
  if (inventory_requested && inventory_complete) {
    addLog(LOG_LEVEL_DEBUG, "DATA : Uploading inventory: " + inventory);
    uploadFile(inventory, "inventory");
    inventory_complete = 0;
    inventory_requested = 0;
  }

  if (!Settings.upload_get) {
    return;
  }

  addLog(LOG_LEVEL_INFO, "DATA : Uploading readings to server");

  String request = "";
  for (int i = 0; i < 10; i++) {
    if (readings.temp[i] != -127) {
      request += "&T" + String(i) + "=" + String(readings.temp[i]);
    }
  }

  if (GPS_present) {
    request += "&GPSdate="    + String(readings.GPS_date);
    request += "&GPStime="    + String(readings.GPS_time);
    request += "&GPSlat="     + String(readings.GPS_lat);
    request += "&GPSlon="     + String(readings.GPS_lon);
    request += "&GPSspeed="   + String(readings.GPS_speed);
    request += "&GPSheading=" + String(readings.GPS_heading);
    request += "&GPSfix="     + String(readings.GPS_fix);
    request += "&GPS_geohash="     + String(readings.GPS_geohash);
  }
  String response;
  if (Settings.upload_get_ssl) {
    response = httpsGet("/update/", request, Settings.upload_get_port);
  } else {
    response = httpGet("/update/", request, Settings.upload_get_port);
  }

  addLog(LOG_LEVEL_DEBUG, "DATA : Response from server: " + response);
}
