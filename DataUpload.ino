void uploadInfluxData() {
  
}

void uploadGetData() {
  addLog(LOG_LEVEL_INFO, "DATA : Uploading readings to server");

  String request = "";
  for(int i=0;i<10;i++) {
    if(readings.temp[i] != -127) {
      request += "&T" + String(i) + "=" + String(readings.temp[i]);  
    }
  }
  
  request += "&Tnk=" + String(readings.Tank_level);
  
  if(BMV_present) { // checksum validation yet to be implemented
    request += "&Ub="  + String(readings.BMV_Vbatt);
    request += "&Um="  + String(readings.BMV_Vaux);
    request += "&Ib="  + String(readings.BMV_Ibatt);
    request += "&Pb="  + String(readings.BMV_Pcharge);
    request += "&SOC=" + String(readings.BMV_SOC);
    request += "&TTG=" + String(readings.BMV_TTG);
    request += "&LDD=" + String(readings.BMV_LDD);
    request += "&CHG=" + String(readings.Charger);
  }
  if(readings.MPPT_ok) {
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
  if(GPS_present) {
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

  if(inventory_requested && inventory_complete) {
    uploadFile(inventory, "inventory");
    inventory_complete = 0;
    inventory_requested = 0;
  }
}
