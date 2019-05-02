String html_head = "<html><head><title>Camper Control</title><style>body {background-color: white; color: black;font-size: 40px;}td {font-size: 40px;}input[type=password], select, input[type=submit] {-webkit-appearance: none;-moz-appearance: none; display: block;margin: 0;width: 100%;height: 56px;line-height: 40px;font-size: 40px;border: 1px solid #bbb;}a, a.vsited {font-size: 15px;text-decoration: none;color: #ffffff;background-color: #005ca9;padding: 10px;display: inline-block;border: 1px solid black;border-radius: 10px;text-transform: uppercase;font-weight: bold;}pre {display: block;background-color: #f0f0f0;border: 1px solid black;font-size: 17px;}</style><meta name='apple-mobile-web-app-capable' content='yes'><meta name='viewport' content='user-scalable=no, initial-scale=.5 width=device-width'><meta charset='UTF-8'></head><body><p><a href='/'>Home</a><a href='/cfg'>WiFi instellingen</a><a href='/bmv'>BMV</a><a href='/mppt'>MPPT</a><a href='/sensors'>MPPT</a><hr>";

void WebServerInit() {
  addLog(LOG_LEVEL_INFO, F("WEB  : Server initializing"));
  
  // Prepare webserver pages
  WebServer.on("/", handle_root);
  WebServer.on("/status", handle_status);
  WebServer.on("/mppt", handle_mppt);
  WebServer.on("/cfg", handle_wificonfig);
  WebServer.on("/savecfg", handle_saveconfig);

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
    content = html_head;
    content += "Laatste meetwaarden:\n";
    content += "<pre>\n";
    content += lastBlockBMV;
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

void handle_mppt() {
  statusLED(true);
  addLog(LOG_LEVEL_DEBUG, F("WEB  : Incoming request for /mppt"));
  if(WiFi.status() == WL_CONNECTED) {
    String content;
    content = html_head;
    content += "Laatste MPPT block:\n";
    content += "<pre>\n";
    content += lastBlockMPPT;
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
  content = html_head;
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
