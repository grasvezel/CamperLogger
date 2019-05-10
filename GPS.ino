void readGPS() {
  String sentence = getGPSsentence();
  int pointer;
  String GPSspeedkt;
  float  deg;
  float  mins;
  float  lat_abs;
  float  lon_abs;

  if (sentence == "") {
    readings.GPS_fix = "T";
    readings.GPS_date = "";
    readings.GPS_time = "";
    readings.GPS_lat = "";
    readings.GPS_lon = "";
    readings.GPS_speed = "";
    readings.GPS_heading = "";
    return;
  }

  // sentence type
  sentence = sentence.substring(7);
  pointer = sentence.indexOf(",");

  // time
  readings.GPS_time = sentence.substring(0, pointer - 3);
  sentence = sentence.substring(pointer + 1);

  // status
  pointer = sentence.indexOf(",");
  readings.GPS_fix = sentence.substring(0, pointer);
  sentence = sentence.substring(pointer + 1);

  // Latitude
  pointer = sentence.indexOf(",");
  readings.GPS_lat = sentence.substring(0, pointer);
  sentence = sentence.substring(pointer + 1);
  deg = int(readings.GPS_lat.toFloat() / 100.0);
  mins = (readings.GPS_lat.toFloat() - deg * 100) / 60.0;
  lat_abs = deg + mins;
  readings.GPS_lat = String(lat_abs, 8);
  pointer = sentence.indexOf(",");
  if (sentence.substring(0, pointer) == "S") {
    lat_abs = 0 - lat_abs;
  }
  readings.GPS_lat_abs = String(lat_abs, 8);
  readings.GPS_lat += sentence.substring(0, pointer);
  sentence = sentence.substring(pointer + 1);

  // Longitude
  pointer = sentence.indexOf(",");
  readings.GPS_lon = sentence.substring(0, pointer);
  sentence = sentence.substring(pointer + 1);
  deg = int(readings.GPS_lon.toFloat() / 100.0);
  mins = (readings.GPS_lon.toFloat() - deg * 100) / 60.0;
  lon_abs = deg + mins;
  readings.GPS_lon = String(lon_abs, 8);
  pointer = sentence.indexOf(",");
  if (sentence.substring(0, pointer) == "W") {
    lon_abs = 0 - lon_abs;
  }
  readings.GPS_lon_abs = String(lon_abs, 8);
  readings.GPS_lon += sentence.substring(0, pointer);
  sentence = sentence.substring(pointer + 1);

  // Speed
  pointer = sentence.indexOf(",");
  GPSspeedkt = sentence.substring(0, pointer);
  float speed = GPSspeedkt.toFloat() * 1.852;
  readings.GPS_speed = String(speed);
  sentence = sentence.substring(pointer + 1);

  // Heading
  pointer = sentence.indexOf(",");
  readings.GPS_heading = sentence.substring(0, pointer);
  sentence = sentence.substring(pointer + 1);
  if (readings.GPS_heading == "") {
    readings.GPS_heading = "0";
  }

  // Date
  pointer = sentence.indexOf(",");
  readings.GPS_date = sentence.substring(0, pointer);
  sentence = sentence.substring(pointer + 1);

  // Done decoding the sentence.
  GPS_present = 1;

  // Now take lat_abs and lon_abs and generate the geohash
  readings.GPS_geohash = geohash(lat_abs, lon_abs);
}

String getGPSsentence() {
  String sentence = "";
  unsigned long timeout = millis() + 2000L;
  while (timeout > millis()) {
    if (SerialGPS.available()) {
      sentence += char(SerialGPS.read());
      if (!sentence.startsWith("$")) {
        sentence = "";
      }
      if (sentence.startsWith("$GPRMC") && sentence.endsWith("\r")) {
        return (sentence);
        sentence = "";
      }
      if (sentence.startsWith("$") && sentence.endsWith("\r")) {
        // uninteresting sentence received.
        sentence = "";
      }
    }
  }
  addLog(LOG_LEVEL_ERROR, "GPS  : Timeout reading GPS module");
  sentence = "";
  return "";
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
