String html_head = "<html><head><title>Camper Control</title><style>body {font-family: verdana;background-color: white; color: black;font-size: 40px;}td,th{font-size: 40px;text-align:left;}input[type=checkbox]{padding:2px;transform:scale(3);}input[type=password],select,input[type=submit],input[type=text],input[type=number]{-webkit-appearance: none;-moz-appearance: none; display: block;margin: 0;width: 100%;height: 56px;line-height: 40px;font-size: 40px;border: 1px solid #bbb;}a, a.vsited {font-size: 15px;text-decoration: none;color: #ffffff;background-color: #005ca9;padding: 10px;display: inline-block;border: 1px solid black;border-radius: 10px;text-transform: uppercase;font-weight: bold;}pre {display: block;background-color: #f0f0f0;border: 1px solid black;font-size: 17px;}</style><meta name='apple-mobile-web-app-capable' content='yes'><meta name='viewport' content='user-scalable=no, initial-scale=.5 width=device-width'><meta charset='UTF-8'></head><body><p><a href='/'>Home</a><a href='/wifi'>WiFi config</a><a href='/cfg'>Settings</a><a href='/bmv'>BMV</a><a href='/mppt'>MPPT</a><a href='/sensors'>Sensors</a><hr>";

void WebServerInit() {
  addLog(LOG_LEVEL_INFO, F("WEB  : Server initializing"));

  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/mppt", handle_mppt);
  WebServer.on("/bmv", handle_bmv);
  WebServer.on("/sensors", handle_sensors);
  WebServer.on("/wifi", handle_wificonfig);
  WebServer.on("/savewifi", handle_savewificonfig);
  WebServer.on("/cfg", handle_cfg);
  WebServer.on("/savecfg", handle_savecfg);
  WebServer.on("/json", handle_json);
  WebServer.on("/reset", ResetFactory);

  WebServer.onNotFound(handle_notfound);

  WebServer.begin();
}

void handle_wificonfig() {
  String content;
  content = html_head;
  addLog(LOG_LEVEL_INFO, "Scanning for networks...");
  // stop trying to connect to WiFi while searching for networks
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_MODE_AP);
  }
  int n = WiFi.scanNetworks();
  addLog(LOG_LEVEL_INFO, "WEB  : Scan done, found " + String(n) + " networks.");
  if (n == 0) {
    content += "No WiFi networks found.";
  } else {
    content += "<p>" + String(n) + " WiFi networks found.</p>";
    content += "<table border=0>";
    content += "<form action=\"/savewifi\" method=\"get\">\n";
    content += "<tr><td>SSID:</td><td><select name=\"ssid\">\n";
    for (int i = 0; i < n; ++i) {
      content += "<option value=\"";
      String ssid = String(WiFi.SSID(i));
      content += ssid + "\">" + int(i + 1) + ": " + ssid + " ";
      content += " (" + String(WiFi.RSSI(i)) + "dBm) "; 
      if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN)
        content += "&#128274;";
      content += "\n";
    }
    content += "</select></td></tr>";
    content += "<tr><td>Key:</td><td><input type=\"password\" name=\"pw\"></td></tr>\n";
    content += "<tr><td>&nbsp;</td><td><input type=\"submit\" value=\"Save\"></td></tr>\n";
    content += "</table>\n";
    content += "</form>";
    content += "</body></html>\n";
  }
  WebServer.send(200, "text/html", content);
}

void handle_savewificonfig() {
  for (int i = 0; i < WebServer.args(); i++) {
    if (WebServer.argName(i) == "ssid")
      WebServer.arg(i).toCharArray(SecuritySettings.WifiSSID, 32);
    if (WebServer.argName(i) == "pw")
      WebServer.arg(i).toCharArray(SecuritySettings.WifiKey, 64);
  }
  addLog(LOG_LEVEL_DEBUG, "WEB  : Config request: SSID: " + String(SecuritySettings.WifiSSID) + " Key: " + String(SecuritySettings.WifiKey));
  addLog(LOG_LEVEL_INFO, "WEB  : Saving settings... " + SaveSettings());

  bool wifi_ok = WifiConnect(3);
  if (wifi_ok) {
    timerAPoff = millis() + 10000L;
  }
  WebServer.sendContent("HTTP/1.1 302\r\nLocation: /\r\n");
}

void handle_root() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /"));
  if (WiFi.status() == WL_CONNECTED) {
    String content;
    content = html_head;
    content += "Connected to WiFi.<br>IP adres: ";
    content += formatIP(WiFi.localIP());
    content += "<br>Time: ";
    content += formattedTime();
    WebServer.send(200, "text/html", content);
    if (timerAPoff != 0)
      timerAPoff = millis() + 10000L;
  } else {
    addLog(LOG_LEVEL_INFO, "WEB  : Not connected to WiFi, redirecting to config page");
    WebServer.sendContent("HTTP/1.1 302\r\nLocation: /wifi\r\n");
  }
  statusLED(false);
}

void handle_mppt() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /mppt"));
  String content;
  content = html_head;
  if (MPPT_present) {
    content += "Last MPPT readings:\n";
    content += "<pre>\n";
    content += lastBlockMPPT;
    content += "</pre>";
  } else {
    content += "No MPPT present";
  }
  WebServer.send(200, "text/html", content);
  if (timerAPoff != 0)
    timerAPoff = millis() + 10000L;
  statusLED(false);
}

void handle_bmv() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /bmv"));
  String content;
  content = html_head;
  if (BMV_present) {
    content += "Last BMV readings:\n";
    content += "<pre>\n";
    content += lastBlockBMV_1;
    content += "\n";
    content += lastBlockBMV_2;
    content += "</pre>";
  } else {
    content += "No BMV present";
  }
  WebServer.send(200, "text/html", content);
  if (timerAPoff != 0)
    timerAPoff = millis() + 10000L;
  statusLED(false);
}

void handle_sensors() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /sensors"));
  String content;
  content = html_head;
  content += "<h2>Temperature sensors:</h2>";
  for (int i = 0; i < 10; i++) {
    if (readings.temp[i] != -127) {
      content += "Sensor " + String(i) + ": " + String(readings.temp[i]) + "&deg;C<br>";
    }
  }
  content += "<h2>Tank sensors:</h2>";
  content += "Water tank " + String(readings.Water_level) + "%<br>";
  content += "Gas tank " + String(readings.Gas_level) + "%<br>";
  WebServer.send(200, "text/html", content);
  if (timerAPoff != 0)
    timerAPoff = millis() + 10000L;
  statusLED(false);
}

void handle_cfg() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /cfg"));
  String content;
  content = html_head;
  content += "<form action=\"/savecfg\" method=\"get\">";
  content += "<table>";
  content += "<tr><th>Influxdb settings</th><th></th></tr>";

  content += "<tr><td>Write data</td><td><input type=\"checkbox\"";
  if (Settings.upload_influx)
    content += " checked";
  content += " name=\"idb_enabled\" value=\"1\"></td></tr>";

  content += "<tr><td>Host</td><td><input type=\"text\" name=\"idb_host\" value=\"" + String(Settings.influx_host) + "\"></td></tr>";
  content += "<tr><td>Port number</td><td><input type=\"number\" name=\"idb_port\" value=\"" + String(Settings.influx_port) + "\"></td></tr>";

  content += "<tr><td>Use SSL</td><td><input type=\"checkbox\"";
  if (Settings.influx_ssl)
    content += " checked";
  content += " name=\"idb_ssl\" value=\"1\"></td></tr>";

  content += "<tr><td>Username</td><td><input type=\"text\" name=\"idb_user\" value=\"" + String(Settings.influx_user) + "\"></td></tr>";
  content += "<tr><td>Password</td><td><input type=\"password\" name=\"idb_pass\" value=\"" + String(Settings.influx_pass) + "\"></td></tr>";
  content += "<tr><td>Database name</td><td><input type=\"text\" name=\"idb_db\" value=\"" + String(Settings.influx_db) + "\"></td></tr>";
  content += "<tr><td>Measurement name</td><td><input type=\"text\" name=\"idb_mn\" value=\"" + String(Settings.influx_mn) + "\"></td></tr>";
  content += "<tr><td>&nbsp;</td><td></td></tr>";
  content += "<tr><th>What to write to influxdb:</th><th></th></tr>";

  content += "<tr><td>BMV readings</td><td><input type=\"checkbox\"";
  if (Settings.influx_write_bmv)
    content += " checked";
  content += " name=\"idb_bmv\" value=\"1\"></td></tr>";

  content += "<tr><td>MPPT readings</td><td><input type=\"checkbox\"";
  if (Settings.influx_write_mppt)
    content += " checked";
  content += " name=\"idb_mppt\" value=\"1\"></td></tr>";

  content += "<tr><td>Temperatures</td><td><input type=\"checkbox\"";
  if (Settings.influx_write_temp)
    content += " checked";
  content += " name=\"idb_temp\" value=\"1\"></td></tr>";

  content += "<tr><td>Water level</td><td><input type=\"checkbox\"";
  if (Settings.influx_write_water)
    content += " checked";
  content += " name=\"idb_tank\" value=\"1\"></td></tr>";

  content += "<tr><td>GPS coords</td><td><input type=\"checkbox\"";
  if (Settings.influx_write_coords)
    content += " checked";
  content += " name=\"idb_coords\" value=\"1\"></td></tr>";

  content += "<tr><td>Geohash</td><td><input type=\"checkbox\"";
  if (Settings.influx_write_geohash)
    content += " checked";
  content += " name=\"idb_geohash\" value=\"1\"></td></tr>";

  content += "<tr><td>Speed / heading</td><td><input type=\"checkbox\"";
  if (Settings.influx_write_speed_heading)
    content += " checked";
  content += " name=\"idb_speed\" value=\"1\"></td></tr>";

  content += "<tr><td>&nbsp;</td><td></td></tr>";
  content += "<tr><th>HTTP GET:</th><th></th></tr>";

  content += "<tr><td>Write data</td><td><input type=\"checkbox\"";
  if (Settings.upload_get)
    content += " checked";
  content += " name=\"get_enabled\" value=\"1\"></td></tr>";

  content += "<tr><td>Host:</td><td><input type=\"text\" name=\"get_host\" value=\"" + String(Settings.upload_get_host) + "\"></td></tr>";
  content += "<tr><td>Port number</td><td><input type=\"number\" name=\"get_port\" value=\"" + String(Settings.upload_get_port) + "\"></td></tr>";

  content += "<tr><td>Use SSL</td><td><input type=\"checkbox\"";
  if (Settings.upload_get_ssl)
    content += " checked";
  content += " name=\"get_ssl\" value=\"1\"></td></tr>";

  content += "<tr><td>&nbsp;</td><td></td></tr>";
  content += "<tr><th>Intervals</th><th></th></tr>";

  content += "<tr><td>Upload interval</td><td><input type=\"number\" name=\"idb_interval\" value=\"" + String(Settings.readings_upload_interval) + "\"></td></tr>";
  content += "<tr><td>GPS upload interval</td><td><input type=\"number\" name=\"gps_interval\" value=\"" + String(Settings.gps_upload_interval) + "\"></td></tr>";

  content += "<tr><td colspan=\"2\"><input type=\"submit\" value=\"Save settings\"></td></tr>";
  content += "</table></form></html>";
  WebServer.send(200, "text/html", content);
  if (timerAPoff != 0)
    timerAPoff = millis() + 10000L;
  statusLED(false);
}

void handle_savecfg() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /savecfg"));
  // Set booleans to 0, we don't get variables from unchecked checkboxes.
  Settings.upload_influx = 0;
  Settings.influx_ssl = 0;
  Settings.upload_get = 0;
  Settings.upload_get_ssl = 0;
  Settings.influx_write_bmv = 0;
  Settings.influx_write_mppt = 0;
  Settings.influx_write_temp = 0;
  Settings.influx_write_water = 0;
  Settings.influx_write_geohash = 0;
  Settings.influx_write_coords = 0;
  Settings.influx_write_speed_heading = 0;

  for (int i = 0; i < WebServer.args(); i++) {
    if (WebServer.argName(i) == "get_enabled" && WebServer.arg(i) == "1") {
      Settings.upload_get = 1;
    }
    if (WebServer.argName(i) == "idb_ssl" && WebServer.arg(i) == "1") {
      Settings.influx_ssl = 1;
    }
    if (WebServer.argName(i) == "idb_enabled" && WebServer.arg(i) == "1") {
      Settings.upload_influx = 1;
    }
    if (WebServer.argName(i) == "idb_bmv" && WebServer.arg(i) == "1") {
      Settings.influx_write_bmv = 1;
    }
    if (WebServer.argName(i) == "idb_mppt" && WebServer.arg(i) == "1") {
      Settings.influx_write_mppt = 1;
    }
    if (WebServer.argName(i) == "idb_temp" && WebServer.arg(i) == "1") {
      Settings.influx_write_temp = 1;
    }
    if (WebServer.argName(i) == "idb_tank" && WebServer.arg(i) == "1") {
      Settings.influx_write_water = 1;
    }
    if (WebServer.argName(i) == "idb_geohash" && WebServer.arg(i) == "1") {
      Settings.influx_write_geohash = 1;
    }
    if (WebServer.argName(i) == "idb_coords" && WebServer.arg(i) == "1") {
      Settings.influx_write_coords = 1;
    }
    if (WebServer.argName(i) == "idb_speed" && WebServer.arg(i) == "1") {
      Settings.influx_write_speed_heading = 1;
    }
    if (WebServer.argName(i) == "get_ssl" && WebServer.arg(i) == "1") {
      Settings.upload_get_ssl = 1;
    }
    if (WebServer.argName(i) == "idb_host") {
      WebServer.arg(i).toCharArray(Settings.influx_host, 64);
    }
    if (WebServer.argName(i) == "idb_db") {
      WebServer.arg(i).toCharArray(Settings.influx_db, 16);
    }
    if (WebServer.argName(i) == "idb_mn") {
      WebServer.arg(i).toCharArray(Settings.influx_mn, 16);
    }
    if (WebServer.argName(i) == "idb_user") {
      WebServer.arg(i).toCharArray(Settings.influx_user, 16);
    }
    if (WebServer.argName(i) == "idb_pass") {
      WebServer.arg(i).toCharArray(Settings.influx_pass, 32);
    }
    if (WebServer.argName(i) == "get_host") {
      WebServer.arg(i).toCharArray(Settings.upload_get_host, 32);
    }
    if (WebServer.argName(i) == "idb_port") {
      Settings.influx_port = WebServer.arg(i).toInt();
    }
    if (WebServer.argName(i) == "get_port") {
      Settings.upload_get_port = WebServer.arg(i).toInt();
    }
    if (WebServer.argName(i) == "gps_interval") {
      Settings.gps_upload_interval = WebServer.arg(i).toInt();
    }
    if (WebServer.argName(i) == "idb_interval") {
      Settings.readings_upload_interval = WebServer.arg(i).toInt();
    }
  }

  String content;
  SaveSettings();
  WebServer.sendContent("HTTP/1.1 302\r\nLocation: /cfg\r\n");
  statusLED(false);
}

void handle_json() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /bmv"));
  String content;
  content  = "{\"bmv\":{";
  content += "\"present\":\"" + String(BMV_present) + "\",";
  content += "\"Vbatt\":\"" + String(readings.BMV_Vbatt) + "\",";
  content += "\"Vaux\":\"" + String(readings.BMV_Vaux) + "\",";
  content += "\"Ibatt\":\"" + String(readings.BMV_Ibatt) + "\",";
  content += "\"SOC\":\"" + String(readings.BMV_SOC) + "\",";
  content += "\"TTG\":\"" + String(readings.BMV_TTG) + "\",";
  content += "\"LDD\":\"" + String(readings.BMV_LDD) + "\",";
  content += "\"PID\":\"" + String(readings.BMV_PID) + "\"";
  content += "},";
  content += "\"mppt\":{";
  content += "\"ytot\":\"" + String(readings.MPPT_ytot) + "\",";
  content += "\"yday\":\"" + String(readings.MPPT_yday) + "\",";
  content += "\"Pmax\":\"" + String(readings.MPPT_Pmax) + "\",";
  content += "\"err\":\"" + String(readings.MPPT_err) + "\",";
  content += "\"state\":\"" + String(readings.MPPT_state) + "\",";
  content += "\"Vbatt\":\"" + String(readings.MPPT_Vbatt) + "\",";
  content += "\"Ibatt\":\"" + String(readings.MPPT_Ibatt) + "\",";
  content += "\"Vpv\":\"" + String(readings.MPPT_Vpv) + "\",";
  content += "\"Ppv\":\"" + String(readings.MPPT_Ppv) + "\",";
  content += "\"PID\":\"" + String(readings.MPPT_PID) + "\",";
  content += "\"serial\":\"" + String(readings.MPPT_serial) + "\"";
  content += "},"; 
  content += "\"gps\":{";
  content += "\"fix\":\"" + String(readings.GPS_fix) + "\",";
  content += "\"date\":\"" + String(readings.GPS_date) + "\",";
  content += "\"time\":\"" + String(readings.GPS_time) + "\",";
  content += "\"lat\":\"" + String(readings.GPS_lat) + "\",";
  content += "\"lat_abs\":\"" + String(readings.GPS_lat_abs) + "\",";
  content += "\"lon\":\"" + String(readings.GPS_lon) + "\",";
  content += "\"lon_abs\":\"" + String(readings.GPS_lon_abs) + "\",";
  content += "\"speed\":\"" + String(readings.GPS_speed) + "\",";
  content += "\"heading\":\"" + String(readings.GPS_heading) + "\",";
  content += "\"geohash\":\"" + String(readings.GPS_geohash) + "\"";
  content += "},"; 
  content += "\"water\":{";
  content += "\"level\":\"" + String(readings.Water_level) + "\"";
  content += "},"; 
  content += "\"gas\":{";
  content += "\"level\":\"" + String(readings.Gas_level) + "\"";
  content += "},"; 
  content += "\"temp\":{";
  int tempsensorcount = 0;
  for(int i=0;i<10;i++) {
    if (readings.temp[i] != -127) {
      tempsensorcount++;
      content += "\"temp" + String(i)+ "\":\"" + String(readings.temp[i]) + "\",";
    }
  }
  if(tempsensorcount > 0) {
    content = content.substring(0, content.length() -1);
  }
  content += "}"; 
  content += "}"; 
  WebServer.send(200, "application/json", content);
  if (timerAPoff != 0)
    timerAPoff = millis() + 10000L;
  statusLED(false);
}

void handle_notfound() {
  String message = "WEB  : File not found: ";
  message += WebServer.uri();
  addLog(LOG_LEVEL_DEBUG, message);
  WebServer.send(404, "text/html", "404 - Not Found");
}
