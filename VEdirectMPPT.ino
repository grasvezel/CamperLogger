void readVEdirectMPPT() {
  String thisBlock;
  char character;
  String line = "";
  int expectedvalues = 9;
  bool block_end = 0;
  bool in_block  = 0;
  // We get a block every second. If we didn't receive
  // anything after two seconds, something is wrong.
  unsigned long vedirect_timeout = millis() + 2000L;

  while (!block_end && !timeOutReached(vedirect_timeout)) {
    while (SerialVE.available() > 0 && !timeOutReached(vedirect_timeout)) {
     vTaskDelay(10 / portTICK_PERIOD_MS); // needed to keep WDT from resetting the ESP
     character = SerialVE.read();
      line += character;
      if (character == '\n') {
        digitalWrite(PIN_EXT_LED, HIGH);
        // line received. Not sure if it is complete yet
        if (line.startsWith("PID")) {
          in_block = 1;
        }
        if (!in_block) {
          // received a line, but we are not in a block and the line does not start with PID. Ignore.
          line = "";
        } else {
          thisBlock += line;
          line.remove(line.length() - 2); // remove <CRLF>

          // Battery voltage
          if (line.startsWith("V\t")) {
            readings.MPPT_Vbatt = line.substring(2).toFloat() / 1000;
            if (readings.MPPT_Vbatt > 3) {
            }
          }

          // Battery current
          if (line.startsWith("I\t")) {
            readings.MPPT_Ibatt = line.substring(2).toFloat() / 1000;
          }

          // PV voltage
          if (line.startsWith("VPV\t")) {
            readings.MPPT_Vpv = line.substring(4).toFloat() / 1000;
          }

          // PV power
          if (line.startsWith("PPV\t")) {
            readings.MPPT_Ppv = line.substring(4).toInt();
          }

          // PV Max power today
          if (line.startsWith("H21\t")) {
            readings.MPPT_Pmax = line.substring(4).toInt();
          }

          // PV yield today
          if (line.startsWith("H20\t")) {
            readings.MPPT_yday = line.substring(4).toFloat() / 100;
          }

          // PV yield total
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

          if (line.startsWith("Checksum")) {
            // End of block
            block_end = 1;
            if(calcChecksum(thisBlock) == 0) {
              // whole block read and checksum is OK.
              lastBlockMPPT = thisBlock;
              // This indcates that a working MPPT controller is present.
              // Until MPPT_present = 1, readings won't be reported.
              MPPT_present = 1;
              readings.MPPT_ok = 1;
              return;
            } else {
              // Checksum on last block was not ok.
              readings.MPPT_ok = 0;
            }
          }
        }
        line = "";
        digitalWrite(PIN_EXT_LED, LOW);
      }
    }
  }
  if (!block_end) {
    // timeout reading ve.direct or at least end of block not found
    addLog(LOG_LEVEL_ERROR, "VICTR: Timeout reading VE.direct (MPPT)");
    return;
  }
}

byte calcChecksum(String input) {
  uint8_t checksum = 0;
  for (int i = 0; i < input.length(); i++) {
    checksum += input.charAt(i);
  }
  return(checksum);
}
