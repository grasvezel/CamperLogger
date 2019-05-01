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

  // Skip this part. We don't have a config file (yet?)
  //  File f = SPIFFS.open(FILE_SETTINGS, "w");
  //  if (f){
  //    for (int x = 0; x < 32768; x++)
  //      f.write(0);
  //    f.close();
  //  }

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

String getFileChecksum(String filename) {
  addLog(LOG_LEVEL_DEBUG, F("FS   : Opening /head.html. If nothing happens after this line, wipe flash using esptool erase_flash."));
  fs::File filehandle = SPIFFS.open(filename, "r+");
  uint16_t crc = 0xFFFF;
  String crc_string;
  if (filehandle) {
    while (filehandle.available()) {
      crc ^= (uint16_t)filehandle.read();  // XOR byte into least sig. byte of crc
      for (int i = 8; i != 0; i--) {    // Loop over each bit
        if ((crc & 0x0001) != 0) {      // If the LSB is set
          crc >>= 1;                    // Shift right and XOR 0xA001
          crc ^= 0xA001;
        } else {                           // Else LSB is not set
          crc >>= 1;                    // Just shift right
        }
      }
    }
    filehandle.close();
    crc_string = String(crc, HEX);
    crc_string.toUpperCase();

    //The crc should be like XXYY. Add zeros if need it
    if (crc_string.length() == 1) {
      crc_string = "000" + crc_string;
    } else if (crc_string.length() == 2) {
      crc_string = "00" + crc_string;
    } else if (crc_string.length() == 3) {
      crc_string = "0" + crc_string;
    }

    //Invert the byte positions
    crc_string = crc_string.substring(2, 4) + crc_string.substring(0, 2);
    return crc_string;
  }
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

void updateHTML(void) {
  return;
  String Scrc = httpsGet("/api/html/checksum/", "");
  if (Scrc.length() != 4) {
    addLog(LOG_LEVEL_ERROR, "FILE: Invalid checksum from server!");
    return;
  }
  //  String Fcrc = getFileChecksum("/head.html");
  addLog(LOG_LEVEL_DEBUG, "FILE : HTML file checksums: " + Fcrc + " (flash), " + Scrc + " (server)");

  if (Fcrc != Scrc) {
    String htmlOnServer = httpsGet("/api/html/datalogger-head.html", "");
    addLog(LOG_LEVEL_INFO, "FILE : Updating HTML on flash");
    File file = SPIFFS.open("/head.html", FILE_WRITE);
    if(!file) {
      addLog(LOG_LEVEL_ERROR, "FILE : Error opening head.html for writing");
    }
    if(file.print(htmlOnServer)) {
      addLog(LOG_LEVEL_INFO, "FILE : HTML file written to SPIFFS");
    } else {
      addLog(LOG_LEVEL_ERROR, "FILE : Error writing HTML file to SPIFFS");
    }
    Fcrc = Scrc; // update CRC without calculating it.
  }
}
