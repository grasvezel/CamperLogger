/********************************************************************************************\
  SPIFFS error handling
  Look here for error # reference: https://github.com/pellepl/spiffs/blob/master/src/spiffs.h
  \*********************************************************************************************/
#define SPIFFS_CHECK(result, fname) if (!(result)) { return(FileError(__LINE__, fname)); }
String FileError(int line, const char * fname) {
  String err("FS   : Error while reading/writing ");
  err = err + fname;
  err = err + " in ";
  err = err + line;
  addLog(LOG_LEVEL_ERROR, err);
  return (err);
}

void fileSystemCheck() {
  addLog(LOG_LEVEL_DEBUG, F("FS   : Mounting SPIFFS"));
  if (SPIFFS.begin()) {
    delay(100);
    addLog(LOG_LEVEL_INFO, "FS   : SPIFFS Mounted");
  } else {
    String log = F("FS   : SPIFFS mount failed");
    addLog(LOG_LEVEL_ERROR, log);
    ResetFactory();
  }
}

void ResetFactory(void) {
  // Perform factory reset and reboot. Not yet implemented
  addLog(LOG_LEVEL_ERROR, "Factory reset.");
  //  addLog(LOG_LEVEL_ERROR, "Not yet implemented");
  //  return;

  //always format on factory reset, in case of corrupt SPIFFS
  SPIFFS.end();
  Serial.println(F("RESET: formatting..."));
  SPIFFS.format();
  Serial.println(F("RESET: formatting done..."));
  if (!SPIFFS.begin()) {
    Serial.println(F("RESET: FORMAT SPIFFS FAILED!"));
    return;
  }

  File settings = SPIFFS.open(FILE_SETTINGS, "w");
  if (settings){
    for (int x = 0; x < 4096; x++)
      settings.write(0);
    settings.close();
  }

  File f = SPIFFS.open(FILE_SECURITY, "w");
  if (f) {
    for (int x = 0; x < 512; x++)
      f.write(0);
    f.close();
  }
  LoadSettings();
  strcpy_P(SecuritySettings.WifiSSID, PSTR(""));
  strcpy_P(SecuritySettings.WifiKey, PSTR(""));
  strcpy_P(SecuritySettings.WifiAPKey, PSTR(""));
  SecuritySettings.Password[0] = 0;
  SaveSettings();
  delay(1000);
  WiFi.persistent(true); // use SDK storage of SSID/WPA parameters
  WiFi.disconnect(); // this will store empty ssid/wpa into sdk storage
  WiFi.persistent(false); // Do not use SDK storage of SSID/WPA parameters
  ESP.restart();
}

String readFile(String filename) {
  fs::File filehandle = SPIFFS.open(filename, "r+");
  String content;
  if (filehandle) {
    while (filehandle.available()) {
      content += (char)filehandle.read();
    }
    filehandle.close();
  }
  return (content);
}

/********************************************************************************************\
  Save settings to SPIFFS
  \*********************************************************************************************/
String SaveSettings(void) {
  SaveToFile((char*)FILE_SETTINGS, 0, (byte*)&Settings, sizeof(struct SettingsStruct));
  return (SaveToFile((char*)FILE_SECURITY, 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct)));
}

/********************************************************************************************\
  Load settings from SPIFFS
  \*********************************************************************************************/
String LoadSettings() {
  LoadFromFile((char*)FILE_SETTINGS, 0, (byte*)&Settings, sizeof(struct SettingsStruct));
  return (LoadFromFile((char*)FILE_SECURITY, 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct)));
}

/********************************************************************************************\
  Save data into config file on SPIFFS
  \*********************************************************************************************/
String SaveToFile(char* fname, int index, byte* memAddress, int datasize) {
  fs::File f = SPIFFS.open(fname, "w");
  SPIFFS_CHECK(f, fname);

  SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
  byte *pointerToByteToSave = memAddress;
  for (int x = 0; x < datasize ; x++)
  {
    SPIFFS_CHECK(f.write(*pointerToByteToSave), fname);
    pointerToByteToSave++;
  }
  f.close();
  String log = F("FILE : Saved ");
  log = log + fname;
  addLog(LOG_LEVEL_INFO, log);

  //OK
  return String();
}

/********************************************************************************************\
  Load data from config file on SPIFFS
  \*********************************************************************************************/
String LoadFromFile(char* fname, int index, byte* memAddress, int datasize)
{
  // addLog(LOG_LEVEL_INFO, String(F("FILE : Load size "))+datasize);

  fs::File f = SPIFFS.open(fname, "r+");
  SPIFFS_CHECK(f, fname);

  // addLog(LOG_LEVEL_INFO, String(F("FILE : File size "))+f.size());

  SPIFFS_CHECK(f.seek(index, fs::SeekSet), fname);
  byte *pointerToByteToRead = memAddress;
  for (int x = 0; x < datasize; x++)
  {
    int readres = f.read();
    SPIFFS_CHECK(readres >= 0, fname);
    *pointerToByteToRead = readres;
    pointerToByteToRead++;// next byte
  }
  f.close();

  return (String());
}

void writeFile(fs::FS &fs, const char * path, String message) {
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    // SPIFFS is potentially open, do not write to it.
    Serial.println("                    Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
  } else {
  }
}
