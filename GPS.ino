String handleGPS() {
  String sentence = getGPSsentence();
  String data = "";
  int pointer;
  String GPStime;
  String GPSstatus;
  String GPSlat;
  String GPSlon;
  String GPSspeedkt;
  String GPSspeedkm;
  String GPSangle;
  String GPSdate;
  float  deg;
  float  mins;
  if(sentence == "") {
    return "&GPSfix=timeout";
  }
  // sentence type
  sentence = sentence.substring(7);
  pointer = sentence.indexOf(",");
  // time
  GPStime = sentence.substring(0,pointer-3);
  sentence = sentence.substring(pointer+1);
  // status
  pointer = sentence.indexOf(",");
  GPSstatus = sentence.substring(0,pointer);
  sentence = sentence.substring(pointer+1);
  // Latitude
  pointer = sentence.indexOf(",");
  GPSlat = sentence.substring(0,pointer);
  sentence = sentence.substring(pointer+1);
  deg = int(GPSlat.toFloat()/100.0);
  mins = (GPSlat.toFloat()-deg * 100)/60.0;
  GPSlat = String(deg+mins,8);
  pointer = sentence.indexOf(",");
  GPSlat += sentence.substring(0,pointer);
  sentence = sentence.substring(pointer+1);
  // Longitude
  pointer = sentence.indexOf(",");
  GPSlon = sentence.substring(0,pointer);
  sentence = sentence.substring(pointer+1);
  deg = int(GPSlon.toFloat()/100.0);
  mins = (GPSlon.toFloat()-deg * 100)/60.0;
  GPSlon = String(deg+mins,8);
  pointer = sentence.indexOf(",");
  GPSlon += sentence.substring(0,pointer);
  sentence = sentence.substring(pointer+1);
  
  // Speed
  pointer = sentence.indexOf(",");
  GPSspeedkt = sentence.substring(0,pointer);
  float speed = GPSspeedkt.toFloat() * 1.852;
  GPSspeedkm = String(speed);
  sentence = sentence.substring(pointer+1);
  // Angle
  pointer = sentence.indexOf(",");
  GPSangle = sentence.substring(0,pointer);
  sentence = sentence.substring(pointer+1);
  if(GPSangle == "") {
    GPSangle = "0";
  }
  // Date
  pointer = sentence.indexOf(",");
  GPSdate = sentence.substring(0,pointer);
  sentence = sentence.substring(pointer+1);
  
//  Serial.println("GPS Time  : " + GPStime);
//  Serial.println("GPS Status: " + GPSstatus);
//  Serial.println("GPS Lat   : " + GPSlat);
//  Serial.println("GPS Lon   : " + GPSlon);
//  Serial.println("GPS Speed : " + GPSspeedkt);
//  Serial.println("GPS Speed : " + GPSspeedkm);
//  Serial.println("GPS Angle : " + GPSangle);
//  Serial.println("GPS Date  : " + GPSdate);

  if(GPSstatus == "A") {
    // fix is good, return data
    data  = "&GPSdate=" + GPSdate;
    data += "&GPStime=" + GPStime;
    data += "&GPSlat=" + GPSlat;
    data += "&GPSlon=" + GPSlon;
    data += "&GPSspeed=" + GPSspeedkm;
    data += "&GPSheading=" + GPSangle;
    data += "&GPSfix=active";
    return data;
  } else {
     // no good fix, don't return data
     return "&GPSfix=void";
  }
}

String getGPSsentence() {
    String sentence = "";
    unsigned long timeout = millis() + 2000L;
    while(timeout > millis()) {
      if(SerialGPS.available()) {
        sentence += char(SerialGPS.read());
        if(!sentence.startsWith("$")) {
          sentence = "";
        }
        if(sentence.startsWith("$GPRMC") && sentence.endsWith("\r")) {
          return(sentence);
          sentence = "";
        }
        if(sentence.startsWith("$") && sentence.endsWith("\r")) {
          // uninteresting sentence received.
          sentence = "";
        }
      }
    }
    addLog(LOG_LEVEL_ERROR, "GPS  : Timeout reading GPS module");
    sentence = "";
    return "";
}

