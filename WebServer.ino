void WebServerInit() {
  addLog(LOG_LEVEL_INFO, F("WEB  : Server initializing"));
  
  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/status", handle_status);
  WebServer.on("/mppttelegram", handle_mppt_telegram);
  WebServer.on("/cfg", handle_wificonfig);
  WebServer.on("/savecfg", handle_saveconfig);

  WebServer.onNotFound(handle_notfound);
  
  WebServer.begin();
}

void handle_wificonfig() {
  String content;
  content = (readFile("/head.html"));
  addLog(LOG_LEVEL_INFO, "Scanning for networks...");
  delay(10);
  int n = WiFi.scanNetworks();
  addLog(LOG_LEVEL_INFO, "WEB  : Scan done, found " + String(n) + " networks.");
  if(n == 0) {
    content += "Geen WiFi netwerken gevonden.";
  } else {
    content += "<p>" + String(n) + " WiFi netwerken gevonden.</p>";
    content += "<table border=0>";
    content += "<form action=\"/savecfg\" method=\"get\">\n";
    content += "<tr><td>SSID:</td><td><select name=\"ssid\">\n";
    for (int i = 0; i < n; ++i) {
      content += "<option value=\"";
      String ssid = String(WiFi.SSID(i));
      content += ssid + "\">" + int(i+1) + ": " + ssid + " ";
      if(WiFi.encryptionType(i) != WIFI_AUTH_OPEN)
        content += "&#128274;";
    content += "\n";
    }
    content += "</select></td></tr>";
    content += "<tr><td>Key:</td><td><input type=\"password\" name=\"pw\"></td></tr>\n";
    content += "<tr><td>&nbsp;</td><td><input type=\"submit\" value=\"Opslaan\"></td></tr>\n";
    content += "</table>\n";
    content += "</form>";
    content += "</body></html>\n";
  }
  WebServer.send(200, "text/html", content);
}

void handle_saveconfig() {
  String message;
  for (int i = 0; i < WebServer.args(); i++) {
    if(WebServer.argName(i) == "ssid")
      WebServer.arg(i).toCharArray(SecuritySettings.WifiSSID,32);
    if(WebServer.argName(i) == "pw")
      WebServer.arg(i).toCharArray(SecuritySettings.WifiKey,64);
  } 
  addLog(LOG_LEVEL_DEBUG, "WEB  : Config request: SSID: " + String(SecuritySettings.WifiSSID) + " Key: " + String(SecuritySettings.WifiKey));

  addLog(LOG_LEVEL_INFO, "WEB  : Saving settings... " + SaveSettings());
  Serial.println(SaveSettings());
  
  bool wifi_ok = WifiConnect(3);
  if(wifi_ok) {
    timerAPoff = millis() + 10000L;
  }
  WebServer.sendContent("HTTP/1.1 302\r\nLocation: /\r\n");
}

void handle_root() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /"));
  if(WiFi.status() == WL_CONNECTED) {
    String content;
    content = readFile("/head.html");
    content += "Laatste meetwaarden:\n";
    content += "<pre>\n";
    content += lastTelegramBMV;
    content += "</pre>";
    WebServer.send(200, "text/html", content);
    if(timerAPoff != 0)
      timerAPoff = millis() + 10000L;
  } else {
    addLog(LOG_LEVEL_INFO, "WEB  : Not connected to WiFi, redirecting to config page");
    WebServer.sendContent("HTTP/1.1 302\r\nLocation: /cfg\r\n");
  }
  statusLED(false);
}

void handle_mppt_telegram() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /mppttelegram"));
  if(WiFi.status() == WL_CONNECTED) {
    String content;
    content = readFile("/head.html");
    content += "Laatste MPPT telegram:\n";
    content += "<pre>\n";
    content += lastTelegramMPPT;
    content += "</pre>";
    WebServer.send(200, "text/html", content);
    if(timerAPoff != 0)
      timerAPoff = millis() + 10000L;
  } else {
    addLog(LOG_LEVEL_INFO, "WEB  : Not connected to WiFi, redirecting to config page");
    WebServer.sendContent("HTTP/1.1 302\r\nLocation: /cfg\r\n");
  }
  statusLED(false);
}

void handle_status() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /telegram"));
  String content;
  content = readFile("/head.html");
  content += "Verbonden met WiFi.<br>IP adres: ";
  content += formatIP(WiFi.localIP());
  content += "<br>Tijd: ";
  content += formattedTime();
  WebServer.send(200, "text/html", content);
  if(timerAPoff != 0)
    timerAPoff = millis() + 10000L;
  statusLED(false);
}


void handle_notfound() {
  String message = "WEB  : File not found: ";
  message += WebServer.uri();
  addLog(LOG_LEVEL_DEBUG, message);
  WebServer.send(404, "text/html", "404 - Not Found");
}

void handle_logo() {
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /logo.png"));
  String logo;
  logo = readFile("/logo.png");
  WebServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  WebServer.sendHeader("Content-Type","image/png",true);
  WebServer.sendContent(logo);
}
