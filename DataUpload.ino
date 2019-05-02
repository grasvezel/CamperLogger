void uploadInfluxData() {
  if (!Settings.upload_influx) {
    return;
  }

  addLog(LOG_LEVEL_INFO, "DATA : Writing data to influxdb");
  
  // this is where the readings are being written to influxdb.
  // You can add or remove any readings below. For MPPT and BMV readings, 
  // check readings.MPPT_ok and readings.BMV_B1_ok respectively to make
  // you are not pushing corrupted data into influxdb. If you are writing
  // readings.BMV_LDD, check readings.BMV_B2_ok (this reading is in the
  // second block of the BMV output).

  if (readings.MPPT_ok) {
    influx_post("Ppv", String(readings.MPPT_Ppv));
    influx_post("Vpv", String(readings.MPPT_Vpv));
  }

  if (readings.BMV_B1_ok) {
    influx_post("24v", String(readings.BMV_Vbatt));
    influx_post("12v", String(readings.BMV_Vaux));
    influx_post("Pcharge", String(readings.BMV_Pcharge));
  }

  if(GPS_present) {
    influx_post("Tank", String(readings.GPS_geohash));
  }
  influx_post("Tank", String(readings.Tank_level));

  for (int i = 0; i < 10; i++) {
    if (readings.temp[i] != -127) {
      influx_post("Temp" + String(i), String(readings.temp[i]));
    }
  }
}

void uploadGetData() {
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

  request += "&Tnk=" + String(readings.Tank_level);

  if (readings.BMV_B1_ok) {
    // These readings are in the first block of the BMV output
    request += "&Ub="  + String(readings.BMV_Vbatt);
    request += "&Um="  + String(readings.BMV_Vaux);
    request += "&Ib="  + String(readings.BMV_Ibatt);
    request += "&Pb="  + String(readings.BMV_Pcharge);
    request += "&SOC=" + String(readings.BMV_SOC);
    request += "&TTG=" + String(readings.BMV_TTG);
    request += "&CHG=" + String(readings.Charger);
  }
  if (readings.BMV_B2_ok) {
    // this reading is in the second block of the BMV output
    request += "&LDD=" + String(readings.BMV_LDD);
  }
  
  if (readings.MPPT_ok) {
    request += "&MUb="    + String(readings.MPPT_Vbatt);
    request += "&MIb="    + String(readings.MPPT_Ibatt);
    request += "&MUpv="   + String(readings.MPPT_Vpv);
    request += "&MPpv="   + String(readings.MPPT_Ppv);
    request += "&Mstate=" + String(readings.MPPT_state);
    request += "&Merr="   + String(readings.MPPT_err);
    request += "&MYtot="  + String(readings.MPPT_ytot);
    request += "&MYt="    + String(readings.MPPT_yday);
    request += "&MPmxt="  + String(readings.MPPT_Pmax);
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

  String response = httpsGet("/update/", request);
  addLog(LOG_LEVEL_DEBUG, "DATA : Response from server: " + response);

  if (inventory_requested && inventory_complete) {
    uploadFile(inventory, "inventory");
    inventory_complete = 0;
    inventory_requested = 0;
  }
}
