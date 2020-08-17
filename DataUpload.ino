void uploadInfluxGPS() {
  if (!Settings.upload_influx) {
    return;
  }
  if (GPS_present) {
    if (Settings.influx_write_geohash) {
      // strings must be quoted
      addLog(LOG_LEVEL_INFO, "DATA : Writing geohash to influxdb");
      influx_post("Geohash", "\"" + String(readings.GPS_geohash) + "\"", "geo");
    }
    if (Settings.influx_write_coords) {
      addLog(LOG_LEVEL_INFO, "DATA : Writing GPS coordinates to influxdb");
      influx_post("lat", String(readings.GPS_lat_abs));
      influx_post("lon", String(readings.GPS_lon_abs));
    }
    if (Settings.influx_write_speed_heading) {
      addLog(LOG_LEVEL_INFO, "DATA : Writing GPS speed & heading to influxdb");
      influx_post("speed", String(readings.GPS_speed));
      influx_post("heading", String(readings.GPS_heading));
    }
  }
}

void uploadInfluxReadings() {
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

  if (readings.MPPT_ok && Settings.influx_write_mppt) {
    influx_post("MPpv",   String(readings.MPPT_Ppv));
    influx_post("MVpv",   String(readings.MPPT_Vpv));
    influx_post("MIb",    String(readings.MPPT_Ibatt));
    influx_post("MVb",    String(readings.MPPT_Vbatt));
    influx_post("Mstate", String(readings.MPPT_state));
    influx_post("Merror", String(readings.MPPT_err));
    influx_post("Mpmax",  String(readings.MPPT_Pmax));
    influx_post("Myday",  String(readings.MPPT_yday));
    influx_post("Mytot",  String(readings.MPPT_ytot));
  }
  // load status and load current are not available on all MPPT's, values should
  // only be written to influxdb if they are present.
  if (readings.MPPT_ok && Settings.influx_write_mppt && readings.MPPT_has_load) {
    influx_post("MIload",  String(readings.MPPT_Iload));
    influx_post("Mload",  String(readings.MPPT_load_on));
  }
  if (readings.BMV_B1_ok && Settings.influx_write_bmv) {
    influx_post("BVbatt", String(readings.BMV_Vbatt));
    influx_post("BVaux",   String(readings.BMV_Vaux));
    influx_post("BPbatt", String(readings.BMV_Pbatt));
    influx_post("BIbatt", String(readings.BMV_Ibatt));
    influx_post("BSOC",   String(readings.BMV_SOC));
    influx_post("BTTG",   String(readings.BMV_TTG));
    influx_post("CHG",    String(readings.Charger));
  }
  if (readings.BMV_B2_ok && Settings.influx_write_bmv) {
    influx_post("BLDD",   String(readings.BMV_LDD));
  }

  if (Settings.influx_write_water) {
    influx_post("Tank", String(readings.Water_level));
  }

  if (Settings.influx_write_gas) {
    influx_post("Gas", String(readings.Gas_level));
  }

  if (Settings.influx_write_temp) {
    for (int i = 0; i < 10; i++) {
      if (readings.temp[i] != -127) {
        influx_post("Temp" + String(i), String(readings.temp[i]));
      }
    }
  }
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

  request += "&Tnk=" + String(readings.Water_level);
  request += "&Gas=" + String(readings.Gas_level);

  if (readings.BMV_B1_ok) {
    // These readings are in the first block of the BMV output
    request += "&Ub="  + String(readings.BMV_Vbatt);
    request += "&Um="  + String(readings.BMV_Vaux);
    request += "&Ib="  + String(readings.BMV_Ibatt);
    request += "&Pb="  + String(readings.BMV_Pbatt);
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
  String response;
  if (Settings.upload_get_ssl) {
    response = httpsGet("/update/", request, Settings.upload_get_port);
  } else {
    response = httpGet("/update/", request, Settings.upload_get_port);
  }

  addLog(LOG_LEVEL_DEBUG, "DATA : Response from server: " + response);
}
