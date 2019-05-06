void OTA() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  char* server = Settings.upload_get_host;
  unsigned long timeoutOTA = millis() + 5000;
  String query;
  unsigned int contentLength = 0;
  int responseCode = 0;
  WiFiClient otaclient;
  int port = 80;

  addLog(LOG_LEVEL_INFO, "OTA  : Starting OTA update");
  if (!otaclient.connect(server, port)) {
    addLog(LOG_LEVEL_ERROR, "OTA  : Connection to OTA server failed");
  }
  addLog(LOG_LEVEL_INFO, "OTA  : Connected to server");
  query = "id=" + String(chipMAC) + "&ver=" + String(version) + "&";
  String url = "http://" + String(server) + "/api/ota/?" + query;
  otaclient.println("GET " + url + " HTTP/1.1");
  otaclient.println("Host: " + String(server));
  otaclient.println("Connection: close");
  otaclient.println();

  // handle timeout
  while (otaclient.available() == 0) {
    if (timeOutReached(timeoutOTA)) {
      addLog(LOG_LEVEL_ERROR, "OTA  : Timeout: no response from server");
      otaclient.stop();
      return;
    }
  }

  // check headers: response code, content type and content length;
  while (otaclient.connected()) {
    String line = otaclient.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    if (line.startsWith("Content-Length: ")) {
      contentLength = line.substring(16).toInt();
      if (contentLength == 0) {
        addLog(LOG_LEVEL_ERROR, "OTA  : New image is 0 bytes. Aborting update");
        otaclient.stop();
        return;
      }
    }
    if (line.startsWith("HTTP/")) {
      responseCode = line.substring(9, 12).toInt();
      if (responseCode != 200) {
        addLog(LOG_LEVEL_ERROR, "OTA  : Server error: " + String(responseCode));
        otaclient.stop();
        return;
      }
    }
    if (line.startsWith("Content-Type: ")) {
      if (! line.startsWith("Content-Type: application/octet-stream")) {
        addLog(LOG_LEVEL_ERROR, "OTA  : Server sends wrong " + line);
        otaclient.stop();
        return;
      }
    }
  }

  // check if there is enough space to store the image
  if (! Update.begin(contentLength)) {
    addLog(LOG_LEVEL_ERROR, "OTA  : Not enough space to store image");
    otaclient.stop();
    return;
  }
  addLog(LOG_LEVEL_INFO, "OTA  : Downloading and installing firmware. This may take a few minutes");
  size_t written = Update.writeStream(otaclient);
  if (written < contentLength) {
    addLog(LOG_LEVEL_ERROR, "OTA  : Only wrote " + String(written) + " bytes, should be " + String(contentLength));
    otaclient.stop();
    return;
  }
  if (Update.end()) {
    addLog(LOG_LEVEL_INFO, "OTA  : Download complete");
    if (Update.isFinished()) {
      addLog(LOG_LEVEL_INFO, "OTA  : Update installed. Rebooting");
      ESP.restart();
    } else {
      addLog(LOG_LEVEL_ERROR, "OTA  : Update not finished. What went wrong?");
    }
  }
}
