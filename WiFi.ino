String WifiGetAPssid() {
  String ssid = "ACC_";
  ssid += String(chipMAC);
  return (ssid);
}

//********************************************************************************
// Set Wifi AP Mode config
//********************************************************************************
void WifiAPconfig() {
//  Disabled softAPConfig as a workaround for this issue:
//  https://github.com/espressif/arduino-esp32/issues/2025
//  WiFi.softAPConfig(apIP, apGW, apNM);
  delay(100);
  WiFi.softAP(WifiGetAPssid().c_str(), DEFAULT_PASSWORD);
  delay(100);
  // We start in AP mode
  WifiAPMode(true);

  String log("WIFI : AP Mode SSID will be ");
  log = log + WifiGetAPssid();

  log = log + F(" with address ");
  log = log + apIP.toString();
  addLog(LOG_LEVEL_INFO, log);
}

//********************************************************************************
// Set Wifi AP Mode
//********************************************************************************
void WifiAPMode(boolean state) {
  if (WifiIsAP()) {
    //want to disable?
    if (!state) {
      WiFi.mode(WIFI_STA);
      addLog(LOG_LEVEL_INFO, F("WIFI : AP Mode disabled"));
    } else {
      addLog(LOG_LEVEL_INFO, F("WIFI : AP Mode already enabled"));
    }
  } else {
    //want to enable?
    if (state) {
      WiFi.mode(WIFI_AP_STA);
      addLog(LOG_LEVEL_INFO, F("WIFI : AP Mode enabled"));
    }
  }
}

bool WifiIsAP() {
  byte wifimode = WiFi.getMode();
  return (wifimode == 2 || wifimode == 3); //apmode is enabled
}

//********************************************************************************
// Configure network and connect to Wifi SSID and SSID2
//********************************************************************************
boolean WifiConnect(byte connectAttempts) {
  String log = "";
  char hostname[40];
  strncpy(hostname, WifiGetAPssid().c_str(), sizeof(hostname));
  WiFi.setHostname(hostname);

  //try to connect to the ap
  if (WifiConnectSSID(SecuritySettings.WifiSSID2,  SecuritySettings.WifiKey2,  connectAttempts)) {
    nextSyncTime = sysTime;
    now();
    return (true);
  } else {
    addLog(LOG_LEVEL_INFO, F("WIFI : tray connect to second wifi"));
    if (WifiConnectSSID(SecuritySettings.WifiSSID,  SecuritySettings.WifiKey,  connectAttempts)) {
       nextSyncTime = sysTime;
       now();
       return (true);
    }
  }

  addLog(LOG_LEVEL_ERROR, F("WIFI : Could not connect to AP!"));
  // Unable to connect to wifi. Enable soft AP.
  WifiAPMode(true);

  return (false);
}

//********************************************************************************
// Connect to Wifi specific SSID
//********************************************************************************
boolean WifiConnectSSID(char WifiSSID[], char WifiKey[], byte connectAttempts)
{
  String log;

  //already connected, need to disconnect first
  if (WiFi.status() == WL_CONNECTED)
    return (true);

  //no ssid specified
  if ((WifiSSID[0] == 0)  || (strcasecmp(WifiSSID, "ssid") == 0))
  {
    addLog(LOG_LEVEL_INFO, F("WIFI : ssid empty!"));
    return (false);
  }

  for (byte tryConnect = 1; tryConnect <= connectAttempts; tryConnect++)
  {
    log = F("WIFI : Connecting ");
    log += WifiSSID;
    log += F(" attempt #");
    log += tryConnect;
    addLog(LOG_LEVEL_INFO, log);

    if (tryConnect == 1)
      WiFi.begin(WifiSSID, WifiKey);
    else
      WiFi.begin();

    //wait until it connects
    for (byte x = 0; x < 200; x++)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        statusLED(false);
        delay(50);
      }
      else
        break;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      //      if (Settings.UseNTP) {
      //        initTime();
      //      }
      log = F("WIFI : Connected! IP: ");
      log += formatIP(WiFi.localIP());
      log += F(" (");
      log += WifiGetAPssid();
      log += F(")");

      addLog(LOG_LEVEL_INFO, log);
      statusLED(true);
      return (true);
    }
    else
    {
      // log = F("WIFI : Disconnecting!");
      // addLog(LOG_LEVEL_INFO, log);
      for (byte x = 0; x < 20; x++)
      {
        statusLED(true);
        delay(50);
      }
    }
  }
  return false;
}

int getWiFiStrength(int points){
    long rssi = 0;
    long averageRSSI=0;
    
    for (int i=0;i < points;i++){
        rssi += WiFi.RSSI();
        delay(20);
    }

   averageRSSI=rssi/points;
    return averageRSSI;
}

void updateAPstatus() {
  // turn off WiFi AP when timeout is reached
  if (timerAPoff != 0 && timeOutReached(timerAPoff)) {
    addLog(LOG_LEVEL_INFO, "WIFI : AP timeout reached");
    timerAPoff = 0;
    WifiAPMode(false);
  }

  // not connected, set timerAPoff to 10 seconds
  if(WiFi.status() != WL_CONNECTED && timerAPoff != 0) {
    timerAPoff = millis() + 10000L;
  }
  
  // trun on WiFi AP if connection is lost
  if(WiFi.status() != WL_CONNECTED && !WifiIsAP()) {
    timerAPoff = millis() + 10000L;
    addLog(LOG_LEVEL_DEBUG, "WIFI : Connection lost, switching on WiFi AP");
    WifiAPMode(true);
  } 
}
