void reportResetReason() {
  if (Settings.upload_get_ssl) {
   httpsGet("/api/reboot/", "&ver=" + String(version,3) + "&Rr0=" + String(rtc_get_reset_reason(0)) + "&Rr1=" + String(rtc_get_reset_reason(1)));
  } else {
   httpGet("/api/reboot/", "&ver=" + String(version,3) + "&Rr0=" + String(rtc_get_reset_reason(0)) + "&Rr1=" + String(rtc_get_reset_reason(1)));
  }
}

void callHome() {
  if (WiFi.status() != WL_CONNECTED) {
    addLog(LOG_LEVEL_ERROR, "WEBCL: Not calling home: Not connected to WiFi");
    return;
  }
  addLog(LOG_LEVEL_INFO, "WEBCL: Calling home");
  String cfgData;
  if (Settings.upload_get_ssl) {
    cfgData = httpsGet("/api/callhome/", "&uptime=" + String(millis() / 1000) + "&free=" + String(system_get_free_heap_size()) + "&rssi=" + getWiFiStrength(10) + "&ip=" + formatIP(WiFi.localIP()), Settings.upload_get_port);
  } else {
    cfgData = httpGet("/api/callhome/", "&uptime=" + String(millis() / 1000) + "&free=" + String(system_get_free_heap_size()) + "&rssi=" + getWiFiStrength(10) + "&ip=" + formatIP(WiFi.localIP()), Settings.upload_get_port);
  }
  String returnValue;
  bool settingsChanged = 0;
  bool returnBool;

  // Process commands from server
  returnValue = getVarFromString("Command:", cfgData);
  if (returnValue != "") {
    if (returnValue.equalsIgnoreCase("reboot")) {
      addLog(LOG_LEVEL_INFO, "CORE : Reboot request from server, rebooting");
      ESP.restart();
    }
    if (returnValue.equalsIgnoreCase("inventory")) {
      addLog(LOG_LEVEL_INFO, "CORE : Inventory request from server");
      inventory_requested = 1;
    }
  }

  // get desired log level from server
  logLevel = getVarFromString("Loglevel:", cfgData).toInt();

  // check DST
  bool WAS_DST = Settings.DST;
  returnValue = getVarFromString("DST:", cfgData);
  if (returnValue == "1") {
    Settings.DST = 1;
    if (WAS_DST == 0) {
      addLog(LOG_LEVEL_INFO, "WEBCL: Server reports it is DST, forwarding the clock 1 hour");
      setTime(sysTime + SECS_PER_HOUR);
      settingsChanged = 1;
    }
  } else {
    Settings.DST = 0;
    if (WAS_DST == 1) {
      addLog(LOG_LEVEL_INFO, "WEBCL: Server reports it is not DST, reversing the clock 1 hour");
      setTime(sysTime - SECS_PER_HOUR);
      settingsChanged = 1;
    }
  }

  if (settingsChanged) {
    addLog(LOG_LEVEL_INFO, "FILE : Settings changed, saving new settings");
    SaveSettings();
  }

  // check version
  returnValue = getVarFromString("Version:", cfgData);
  if (returnValue != "") {
    float srvVer = returnValue.toFloat();
    if (srvVer == 0) {
      addLog(LOG_LEVEL_INFO, "OTA  : No software available from server");
      return;
    }
    addLog(LOG_LEVEL_DEBUG, "OTA  : SW version available on server: " + String(srvVer,3));
    if (version == srvVer) {
      addLog(LOG_LEVEL_INFO, "OTA  : SW version up to date");
    }
    if (version < srvVer) {
      addLog(LOG_LEVEL_INFO, "OTA  : New version available");
      OTA();
    }
    if (version > srvVer && srvVer > 1) {
      addLog(LOG_LEVEL_INFO, "OTA  : Forced downgrade");
      OTA();
    }
  }
}

String getVarFromString(String var, String cfgData) {
  String line;
  String curChar;
  String foundValue;
  int datalen = cfgData.length();
  for (int i = 0; i < datalen; i++) {
    curChar = cfgData.substring(i, i + 1);
    if (curChar != "\n" && curChar != "\r") {
      line += curChar;
    } else {
      if (line.startsWith(var)) {
        foundValue = line.substring(var.length());
        foundValue.trim();
        return (foundValue);
      }
      line = "";
    }
  }
  return ("");
}

String urlOpen(String path, String query) {
  if (path.startsWith("https://")) {
    return (httpsGet(path, query));
  } else if (path.startsWith("http://")) {
    return (httpGet(path, query));
  } else {
    addLog(LOG_LEVEL_ERROR, "WEBCL: Unsupported URL: " + path);
  }
}

String httpsGet(String path, String query, int port) {
  if (WiFi.status() != WL_CONNECTED) {
    addLog(LOG_LEVEL_ERROR, "WEBCL: Not trying to connect: Not connected to WiFi");
    return("");
  }
  WiFiClientSecure webclient;
  webclient.setTimeout(3);
  addLog(LOG_LEVEL_DEBUG, "WEBCL: Starting https request (" + path + ")" );
  unsigned int contentLength = 0;
  unsigned int contentPos = 0;
  String response = "";
  String responseCode = "";
  char* server = Settings.upload_get_host;
  query = "id=" + String(chipMAC) + query;
  String url = "https://" + String(server) + path + "?" + query;
  if (!webclient.connect(server, port))
    addLog(LOG_LEVEL_INFO, "WEBCL: Connection to " + String(server) + " failed!");
  else {
    addLog(LOG_LEVEL_DEBUG, "WEBCL: Connected to " + String(server));
    webclient.println("GET " + url + " HTTP/1.1");
    webclient.println("Host: " + String(server));
    webclient.println("Connection: close");
    webclient.println();
    String line;
    while (webclient.connected()) {
      line = webclient.readStringUntil('\n');
      if (line == "\r") {
        break;
      }
      if (line.startsWith("Content-Length: ")) {
        contentLength = line.substring(16).toInt();
      }
      if (line.startsWith("HTTP/")) {
        responseCode = line.substring(9, 12);
      }
    }
    line = "";
    while (webclient.available() && contentPos < contentLength) {
      char c = webclient.read();
      contentPos++;
      response += String(c);
    }
    webclient.stop();
  }
  if (responseCode == "200") {
    addLog(LOG_LEVEL_DEBUG, "WEBCL: Request done ");
    return (response);
  } else {
    addLog(LOG_LEVEL_ERROR, "WEBCL: Response code: " + String(responseCode));
    return responseCode;
  }
}

String httpGet(String path, String query, int port) {
  if (WiFi.status() != WL_CONNECTED) {
    addLog(LOG_LEVEL_ERROR, "WEBCL: Not trying to connect: Not connected to WiFi");
    return("");
  }
  WiFiClient webclient;
  webclient.setTimeout(3);
  addLog(LOG_LEVEL_DEBUG, "WEBCL: Starting https request (" + path + ")" );
  unsigned int contentLength = 0;
  unsigned int contentPos = 0;
  String response = "";
  String responseCode = "";
  char* server = Settings.upload_get_host;
  query = "id=" + String(chipMAC) + "&" + query;
  String url = "http://" + String(server) + path + "?" + query;
  if (!webclient.connect(server, port))
    addLog(LOG_LEVEL_INFO, "WEBCL: Connection to " + String(server) + " failed!");
  else {
    addLog(LOG_LEVEL_DEBUG, "WEBCL: Connected to " + String(server));
    webclient.println("GET " + url + " HTTP/1.1");
    webclient.println("Host: " + String(server));
    webclient.println("Connection: close");
    webclient.println();
    String line;
    while (webclient.connected()) {
      line = webclient.readStringUntil('\n');
      if (line == "\r") {
        break;
      }
      if (line.startsWith("Content-Length: ")) {
        contentLength = line.substring(16).toInt();
      }
      if (line.startsWith("HTTP/")) {
        responseCode = line.substring(9, 12);
      }
    }
    line = "";
    while (webclient.available() && contentPos < contentLength) {
      char c = webclient.read();
      contentPos++;
      response += String(c);
    }
    webclient.stop();
  }
  if (responseCode == "200") {
    return (response);
  } else {
    addLog(LOG_LEVEL_ERROR, "WEBCL: Response code: " + String(responseCode));
    return responseCode;
  }
}

void uploadFile(String content, String type) {
  String url = "/api/upload/?id=" + String(chipMAC) + "&file=" + type;
  char* server = Settings.upload_get_host;
  char bitbucket;
  WiFiClientSecure uploadclient;
  if (uploadclient.connect(server, Settings.upload_get_port)) {
    // Log to console only, the logfile is opened!
    uint32_t fileSize = content.length();
    uploadclient.println("POST " + url + " HTTP/1.1");
    uploadclient.println("Host: " + String(server));
    uploadclient.println("Connection: close");
    uploadclient.println("Content-type: text/plain");
    uploadclient.println("Content-length: " + String(fileSize));
    uploadclient.println();

    // send file contents to webserver
    uploadclient.print(content);
    uploadclient.stop();
  }
}

void influx_post(String var, String value, String field) {
  String postVars = String(Settings.influx_mn) + ",item=" + var + ",logger=" + String(chipMAC) + " " + field + "=" + value;
  vTaskDelay(10 / portTICK_PERIOD_MS);

  if (Settings.influx_ssl) {
    addLog(LOG_LEVEL_ERROR, "WEBCL: write using SSL to host" + String(Settings.influx_host) + " port:" + String(Settings.influx_port));
    WiFiClientSecure client;
    if (!client.connect(Settings.influx_host, Settings.influx_port)) {
      addLog(LOG_LEVEL_ERROR, "WEBCL: connection failed");
      return;
    }
    addLog(LOG_LEVEL_ERROR, "WEBCL: write do db:" + String(Settings.influx_db));
    client.println("POST /write?db=" + String(Settings.influx_db) + " HTTP/1.1");
    client.println("Host: " + String(Settings.influx_host));
    client.println("Authorization: Basic " + base64::encode(String(Settings.influx_user) + ":" + String(Settings.influx_pass)));
    client.println("Connection: close");
    client.println("Content-length: " + String(postVars.length()));
    client.println();
    client.println(postVars);
    client.stop();
    addLog(LOG_LEVEL_ERROR, "WEBCL: write finished");
  } else {
    addLog(LOG_LEVEL_ERROR, "WEBCL: write using HTTP");
    WiFiClient client;
    if (!client.connect(Settings.influx_host, Settings.influx_port)) {
      addLog(LOG_LEVEL_ERROR, "WEBCL: connection failed");
      return;
    }
    client.println("POST /write?db=" + String(Settings.influx_db) + " HTTP/1.1");
    client.println("Host: " + String(Settings.influx_host));
    client.println("Authorization: Basic " + base64::encode(String(Settings.influx_user) + ":" + String(Settings.influx_pass)));
    client.println("Connection: close");
    client.println("Content-length: " + String(postVars.length()));
    client.println();
    client.println(postVars);
    client.stop();
    addLog(LOG_LEVEL_ERROR, "WEBCL: write finished");
  }
}
