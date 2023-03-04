void readGPS() {
  unsigned long timeout = millis() + 4000L;
  while (timeout > millis()) {
    while (SerialGPS.available() > 0){
      gpsParser.encode(SerialGPS.read());
    }
    readings.GPS_sat = gpsParser.satellites.value();
    if (gpsParser.location.isValid()) {
      GPS_present = 1;
      readings.GPS_fix = "T";
      readings.GPS_date = gpsParser.date.value();
      readings.GPS_time = gpsParser.time.value();
      readings.GPS_lat = String(gpsParser.location.lat(),8);
      readings.GPS_lon = String(gpsParser.location.lng(),8);
      readings.GPS_alt = gpsParser.altitude.meters();
      readings.GPS_sat = gpsParser.satellites.value();
      readings.GPS_dop = String(gpsParser.hdop.value(),5);
      readings.GPS_geohash = geohash(gpsParser.location.lat(), gpsParser.location.lng());

      // printRawGPSData();
      return;
    } else {
      GPS_present = 0;      
    }
  }
  addLog(LOG_LEVEL_ERROR, "GPS  : Timeout reading GPS module");

}

String geohash (float lat, float lng) {
  // I borrowed this code from Dennis Geurts (github.com/dennisg). Thanks, Dennis.

  char base32[] = "0123456789bcdefghjkmnpqrstuvwxyz";
  byte precision = GEOHASH_PRECISION;
  char hash[precision + 1];

  uint32_t LatitudeBinary  = ((lat  +  90.0) * 0xFFFFFFFF / 180.0);
  uint32_t LongitudeBinary = ((lng  + 180.0) * 0xFFFFFFFF / 360.0);

  uint32_t latMin = 0, latMax =  0xFFFFFFFF;
  uint32_t lonMin = 0, lonMax =  0xFFFFFFFF;

  uint8_t i   = 0; // index into geohash
  uint8_t idx = 0; // index into base32 map
  uint8_t bit = 0; // each char holds 5 bits
  boolean evenBit = true;

  while (i <= precision) {
    if (evenBit) {
      // bisect E-W longitude
      uint32_t lonMid = (lonMin / 2 + lonMax / 2);

      if (LongitudeBinary >= lonMid) {
        idx = idx * 2 + 1;
        lonMin = lonMid;
      } else {
        idx = idx * 2;
        lonMax = lonMid;
      }
    } else {
      // bisect N-S latitude
      uint32_t latMid = (latMin / 2 + latMax / 2);

      if (LatitudeBinary >= latMid) {
        idx = idx * 2 + 1;
        latMin = latMid;
      } else {
        idx = idx * 2;
        latMax = latMid;
      }
    }
    evenBit = !evenBit;

    if (++bit == 5) {
      // 5 bits gives us a character: append it and start over
      hash[i++] = base32[idx];
      bit = 0;
      idx = 0;
    }
  }

  hash[precision] = 0;
  return String(hash);
}

void printRawGPSData() {
  Serial.println(gpsParser.altitude.meters());
  Serial.print("LAT=");  Serial.println(gpsParser.location.lat(), 6);
  Serial.print("LONG="); Serial.println(gpsParser.location.lng(), 6);
  Serial.print("ALT=");  Serial.println(gpsParser.altitude.meters());
  Serial.println(gpsParser.location.lat(), 6); // Latitude in degrees (double)
  Serial.println(gpsParser.location.lng(), 6); // Longitude in degrees (double)
  Serial.print(gpsParser.location.rawLat().negative ? "-" : "+");
  Serial.println(gpsParser.location.rawLat().deg); // Raw latitude in whole degrees
  Serial.println(gpsParser.location.rawLat().billionths);// ... and billionths (u16/u32)
  Serial.print(gpsParser.location.rawLng().negative ? "-" : "+");
  Serial.println(gpsParser.location.rawLng().deg); // Raw longitude in whole degrees
  Serial.println(gpsParser.location.rawLng().billionths);// ... and billionths (u16/u32)
  Serial.println(gpsParser.date.value()); // Raw date in DDMMYY format (u32)
  Serial.println(gpsParser.date.year()); // Year (2000+) (u16)
  Serial.println(gpsParser.date.month()); // Month (1-12) (u8)
  Serial.println(gpsParser.date.day()); // Day (1-31) (u8)
  Serial.println(gpsParser.time.value()); // Raw time in HHMMSSCC format (u32)
  Serial.println(gpsParser.time.hour()); // Hour (0-23) (u8)
  Serial.println(gpsParser.time.minute()); // Minute (0-59) (u8)
  Serial.println(gpsParser.time.second()); // Second (0-59) (u8)
  Serial.println(gpsParser.time.centisecond()); // 100ths of a second (0-99) (u8)
  Serial.println(gpsParser.speed.value()); // Raw speed in 100ths of a knot (i32)
  Serial.println(gpsParser.speed.knots()); // Speed in knots (double)
  Serial.println(gpsParser.speed.mph()); // Speed in miles per hour (double)
  Serial.println(gpsParser.speed.mps()); // Speed in meters per second (double)
  Serial.println(gpsParser.speed.kmph()); // Speed in kilometers per hour (double)
  Serial.println(gpsParser.course.value()); // Raw course in 100ths of a degree (i32)
  Serial.println(gpsParser.course.deg()); // Course in degrees (double)
  Serial.println(gpsParser.altitude.value()); // Raw altitude in centimeters (i32)
  Serial.print("ALT(m)=");  Serial.println(gpsParser.altitude.meters()); // Altitude in meters (double)
  Serial.println(gpsParser.altitude.miles()); // Altitude in miles (double)
  Serial.println(gpsParser.altitude.kilometers()); // Altitude in kilometers (double)
  Serial.println(gpsParser.altitude.feet()); // Altitude in feet (double)
  Serial.print("SAT=");  Serial.println(gpsParser.satellites.value()); // Number of satellites in use (u32)
  Serial.print("DOP=");  Serial.println(gpsParser.hdop.value()); // Horizontal Dim. of Precision (100ths-i32)
}
