String html_head = "<html><head><title>Camper Control</title><style>body {background-color: white; color: black;font-size: 40px;}td {font-size: 40px;}input[type=checkbox]{padding:2px;transform:scale(3);}input[type=password],select,input[type=submit],input[type=text],input[type=number]{-webkit-appearance: none;-moz-appearance: none; display: block;margin: 0;width: 100%;height: 56px;line-height: 40px;font-size: 40px;border: 1px solid #bbb;}a, a.vsited {font-size: 15px;text-decoration: none;color: #ffffff;background-color: #005ca9;padding: 10px;display: inline-block;border: 1px solid black;border-radius: 10px;text-transform: uppercase;font-weight: bold;}pre {display: block;background-color: #f0f0f0;border: 1px solid black;font-size: 17px;}</style><meta name='apple-mobile-web-app-capable' content='yes'><meta name='viewport' content='user-scalable=no, initial-scale=.5 width=device-width'><meta charset='UTF-8'></head><body><p><a href='/'>Home</a><a href='/wifi'>WiFi config</a><a href='/cfg'>Settings</a><a href='/bmv'>BMV</a><a href='/mppt'>MPPT</a><a href='/sensors'>Sensors</a><hr>";

void WebServerInit() {
  addLog(LOG_LEVEL_INFO, F("WEB  : Server initializing"));

  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/mppt", handle_mppt);
  WebServer.on("/bmv", handle_bmv);
  WebServer.on("/sensors", handle_sensors);
  WebServer.on("/wifi", handle_wificonfig);
  WebServer.on("/savewif", handle_savewificonfig);
  WebServer.on("/cfg", handle_cfg);
  WebServer.on("/savecfg", handle_savecfg);

  WebServer.onNotFound(handle_notfound);

  WebServer.begin();
}

void handle_wificonfig() {
  String content;
  content = html_head;
  addLog(LOG_LEVEL_INFO, "Scanning for networks...");
  delay(10);
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
  String message;
  for (int i = 0; i < WebServer.args(); i++) {
    if (WebServer.argName(i) == "ssid")
      WebServer.arg(i).toCharArray(SecuritySettings.WifiSSID, 32);
    if (WebServer.argName(i) == "pw")
      WebServer.arg(i).toCharArray(SecuritySettings.WifiKey, 64);
  }
  addLog(LOG_LEVEL_DEBUG, "WEB  : Config request: SSID: " + String(SecuritySettings.WifiSSID) + " Key: " + String(SecuritySettings.WifiKey));

  addLog(LOG_LEVEL_INFO, "WEB  : Saving settings... " + SaveSettings());
  Serial.println(SaveSettings());

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
  content += "Last MPPT readings:\n";
  content += "<pre>\n";
  content += lastBlockMPPT;
  content += "</pre>";
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
  content += "Last BMV readings:\n";
  content += "<pre>\n";
  content += lastBlockBMV;
  content += "</pre>";
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
  content += "Last sensor readings:\n";
  content += "<pre>\n";
  content += "Not yet implemented.";
  content += "</pre>";
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
  content += "<h3>Influxdb settings</h3>";
  content += "Influxdb host: <input type=\"text\" name=\"idb_host\" value=\""+ Settings.influx_host + "\"><br>";
  content += "Influxdb username: <input type=\"text\" name=\"idb_user\" value=\""+ Settings.influx_user + "\"><br>";
  content += "Influxdb password: <input type=\"password\" name=\"idb_pass\" value=\""+ Settings.influx_pass + "\"><br>";
  content += "Influxdb database name: <input type=\"text\" name=\"idb_db\" value=\""+ Settings.influx_db + "\"><br>";
  content += "Influxdb measurement name: <input type=\"text\" name=\"idb_mn\" value=\""+ Settings.influx_mn + "\"><br>";
  content += "<h3>What to write to influxdb:</h3>";
  content += "&emsp;<input type=\"checkbox\" value=\"idb_bmv\">&emsp;BMV readings<br>";
  content += "&emsp;<input type=\"checkbox\" value=\"idb_mppt\">&emsp;MPPT readings<br>";
  content += "&emsp;<input type=\"checkbox\" value=\"idb_temp\">&emsp;Temperatures<br>";
  content += "&emsp;<input type=\"checkbox\" value=\"idb_tank\">&emsp;Tank level<br>";
  content += "&emsp;<input type=\"checkbox\" value=\"idb_coords\">&emsp;GPS coords<br>";
  content += "&emsp;<input type=\"checkbox\" value=\"idb_geohash\">&emsp;GPS geohash<br>";
  content += "&emsp;<input type=\"checkbox\" value=\"idb_speed\">&emsp;GPS speed and heading<br>";
  content += "<h3>Intervals</h3>";
  content += "Seconds between measurement uploads: <input type=\"number\" name=\"idb_interval\"><br>";
  content += "Seconds between GPS uploads: <input type=\"number\" name=\"idb_interval\"><br>";
  
  content += "<p><input type=\"submit\" value=\"Save settings\">";
  content += "</form></html>";
  WebServer.send(200, "text/html", content);
  if (timerAPoff != 0)
    timerAPoff = millis() + 10000L;
  statusLED(false);
}

void handle_savecfg() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /savecfg"));
  String content;
  content = html_head;
  WebServer.send(200, "text/html", content);
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
