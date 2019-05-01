void readVEdirect(int device) {
  String thisBlock;
  char character;
  String line = "";
  bool block_end = 0;
  bool in_block  = 0;
  // We get a block every second. However, a BMV sends different odd and even blocks.
  // This means it may take a little over two seconds to get to the block we want. 
  unsigned long vedirect_timeout = millis() + 4000L; //

  while (!block_end && !timeOutReached(vedirect_timeout)) {
    while (SerialVE.available() > 0 && !timeOutReached(vedirect_timeout)) {
      vTaskDelay(10 / portTICK_PERIOD_MS); // needed to keep WDT from resetting the ESP
      character = SerialVE.read();
      line += character;
      if (character == '\n') {
        digitalWrite(PIN_EXT_LED, HIGH);
        // line received. Not sure if it is complete yet
        if (line.startsWith("PID") || (device == DEVICE_BMV_B2 && line.startsWith("H1\t"))) {  // second part of BMV block starts with H1
          in_block = 1;
        }
        if (!in_block) {
          // received a line, but we are not in a block and the line does not start with PID. Ignore.
          line = "";
        } else {
          vTaskDelay(10 / portTICK_PERIOD_MS);
          thisBlock += line;
          line.remove(line.length() - 2); // remove <CRLF>

          if (device == DEVICE_MPPT) {
            parseMPPT(line);
          }
          if (device == DEVICE_BMV_B1 || device == DEVICE_BMV_B2) {
            parseBMV(line);
          }

          // checksum validation
          if (line.startsWith("Checksum")) {
            // End of block
            block_end = 1;
            vTaskDelay(10 / portTICK_PERIOD_MS);
            if (calcChecksum(thisBlock) == 0) { // checksum should always be 0
              if (device == DEVICE_MPPT) {
                lastBlockMPPT = thisBlock;
                MPPT_present = 1;
                readings.MPPT_ok = 1;
                addLog(LOG_LEVEL_INFO, "VICTR: Checksum OK reading device " + String(device));
              }
              if (device == DEVICE_BMV_B1) {
                lastBlockBMV = thisBlock;
                BMV_present = 1;
                readings.BMV_B1_ok = 1;
                addLog(LOG_LEVEL_INFO, "VICTR: Checksum OK reading device " + String(device));
              }
              if (device == DEVICE_BMV_B2) {
                lastBlockBMV += thisBlock;
                BMV_present = 1;
                readings.BMV_B2_ok = 1;
                addLog(LOG_LEVEL_INFO, "VICTR: Checksum OK reading device " + String(device));
              }
              return;
            } else {
              if (device == DEVICE_MPPT) {
                readings.MPPT_ok = 0;
                addLog(LOG_LEVEL_ERROR, "VICTR: Checksum error reading device " + String(device));
                lastBlockMPPT = thisBlock + "Invalid checksum BMV block 1";
              }
              if (device == DEVICE_BMV_B1) {
                readings.BMV_B1_ok = 0;
                addLog(LOG_LEVEL_ERROR, "VICTR: Checksum error reading device " + String(device));
                lastBlockBMV = thisBlock + "Invalid checksum BMV block 1";
              }
              if (device == DEVICE_BMV_B2) {
                readings.BMV_B2_ok = 0;
                addLog(LOG_LEVEL_ERROR, "VICTR: Checksum error reading device " + String(device));
                lastBlockBMV = thisBlock + "Invalid checksum BMV block 2";                
              }
            }
          }
        }
        line = "";
        digitalWrite(PIN_EXT_LED, LOW);
      }
    }
  }
  if (!block_end) {
    // timeout reading MPPT or at least end of block not found
    addLog(LOG_LEVEL_ERROR, "VICTR: Timeout reading VE.direct device " + String(device));
    return;
  }
}

byte calcChecksum(String input) {
  uint8_t checksum = 0;
  for (int i = 0; i < input.length(); i++) {
    checksum += input.charAt(i);
  }
  return (checksum);
}


void parseMPPT(String line) {
  // MPPT output (battery) voltage
  if (line.startsWith("V\t")) {
    readings.MPPT_Vbatt = line.substring(2).toFloat() / 1000;
    if (readings.MPPT_Vbatt > 3) {
    }
  }

  // MPPT output (battery) current
  if (line.startsWith("I\t")) {
    readings.MPPT_Ibatt = line.substring(2).toFloat() / 1000;
  }

  // MPTT input (PV) voltage
  if (line.startsWith("VPV\t")) {
    readings.MPPT_Vpv = line.substring(4).toFloat() / 1000;
  }

  // MPPT input (PV) power
  if (line.startsWith("PPV\t")) {
    readings.MPPT_Ppv = line.substring(4).toInt();
  }

  // MPPT max power today
  if (line.startsWith("H21\t")) {
    readings.MPPT_Pmax = line.substring(4).toInt();
  }

  // MPPT PV yield today
  if (line.startsWith("H20\t")) {
    readings.MPPT_yday = line.substring(4).toFloat() / 100;
  }

  // MPPT PV yield total
  if (line.startsWith("H19\t")) {
    readings.MPPT_ytot = line.substring(4).toFloat() / 100;
  }

  // MPPT error number
  if (line.startsWith("ERR\t")) {
    readings.MPPT_err = line.substring(4).toInt();
  }

  // MPPT charge state
  if (line.startsWith("CS\t")) {
    readings.MPPT_state = line.substring(3).toInt();
  }
}

void parseBMV(String line) {
  // BMV battery voltage
  if (line.startsWith("V\t")) {
    readings.BMV_Vbatt = line.substring(2).toFloat() / 1000;
  }

  // BMV auxilary battery voltage
  if (line.startsWith("VS\t") || line.startsWith("VM\t")) {
    readings.BMV_Vaux = line.substring(3).toFloat() / 1000;
  }

  // BMV State Of Charge
  if (line.startsWith("SOC\t")) {
    readings.BMV_SOC = line.substring(4).toFloat() / 10;
  }
  
  // BMV battery current
  if (line.startsWith("I\t")) {
    readings.BMV_Ibatt = line.substring(2).toFloat() / 1000;
  }

  // BMV Time To Go
  if (line.startsWith("TTG\t")) {
    readings.BMV_TTG = line.substring(4).toFloat();
  }

  // BMV Last Discharge Depth
  if (line.startsWith("H2\t")) {
    readings.BMV_LDD = line.substring(3).toFloat() / 1000;
  }

  // BMV battery power
  if (line.startsWith("P\t")) {
    readings.BMV_Pcharge = line.substring(2).toFloat();
  }
}
