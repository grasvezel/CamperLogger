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
    request += "&GPSdate="     + String(readings.GPS_date);
    request += "&GPStime="     + String(readings.GPS_time);
    request += "&GPSlat="      + String(readings.GPS_lat);
    request += "&GPSlon="      + String(readings.GPS_lon);
    request += "&GPSspeed="    + String(readings.GPS_speed);
    request += "&GPSheading="  + String(readings.GPS_heading);
    request += "&GPSfix="      + String(readings.GPS_fix);
    request += "&GPS_geohash=" + String(readings.GPS_geohash);
  }
  String response;
  if (Settings.upload_get_ssl) {
    response = httpsGet("/update/", request, Settings.upload_get_port);
  } else {
    response = httpGet("/update/", request, Settings.upload_get_port);
  }

  addLog(LOG_LEVEL_DEBUG, "DATA : Response from server: " + response);
}
