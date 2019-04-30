void readVEdirectMPPT() {
  String thisTelegram;
  int valuesread = 0;
  char character;
  char prev_character;
  String line = "";
  // We get a telegram every second. If we didn't receive
  // anything after two seconds, something is wrong.
  unsigned long vedirect_timeout = millis() + 2000L;
  int expectedvalues = 9;

  // Try to find the beginning of the telegram (empty line)
  // so we can start reading from the beginning.
  while (SerialVE.available() > 0 && !timeOutReached(vedirect_timeout)) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    character = SerialVE.read();
    if (character == '\n' && prev_character == '\n') {
      break;
    }
    prev_character = character;
  }
  
  while (valuesread < expectedvalues && !timeOutReached(vedirect_timeout)) {
    while (SerialVE.available() > 0 && !timeOutReached(vedirect_timeout)) {
      digitalWrite(PIN_EXT_LED, HIGH);
      character = SerialVE.read();
      thisTelegram += character;
      if (character == '\n') {
        // We got a complete line. 

        // Battery voltage
        if (line.startsWith("V\t")) {
          readings.MPPT_Vbatt = line.substring(2).toFloat() / 1000;
          if (readings.MPPT_Vbatt > 3) {
            valuesread++;
          }
        }

        // Battery current
        if (line.startsWith("I\t")) {
          readings.MPPT_Ibatt = line.substring(2).toFloat() / 1000;
          valuesread++;
        }

        // PV voltage
        if (line.startsWith("VPV\t")) {
          readings.MPPT_Vpv = line.substring(4).toFloat() / 1000;
          valuesread++;
        }

        // PV power
        if (line.startsWith("PPV\t")) {
          valuesread++;
          readings.MPPT_Ppv = line.substring(4).toInt();
        }

        // PV Max power today
        if (line.startsWith("H21\t")) {
          valuesread++;
          readings.MPPT_Pmax = line.substring(4).toInt();
        }

        // PV yield today
        if (line.startsWith("H20\t")) {
          valuesread++;
          readings.MPPT_yday = line.substring(4).toFloat() / 100;
        }

        // PV yield total
        if (line.startsWith("H19\t")) {
          valuesread++;
          readings.MPPT_ytot = line.substring(4).toFloat() / 100;
        }

        // MPPT error number
        if (line.startsWith("ERR\t")) {
          valuesread++;
          readings.MPPT_err = line.substring(4).toInt();
        }

        // MPPT charge state
        if (line.startsWith("CS\t")) {
          valuesread++;
          readings.MPPT_state = line.substring(3).toInt();
        }

        line = "";
      } else {
        line += character;
      }
    }
    digitalWrite(PIN_EXT_LED, LOW);
  }

  
  // TODO: rather than counting values, we should find the end of the telegram
  
  if (valuesread == expectedvalues) {    
    lastTelegramMPPT = thisTelegram;
    MPPT_present = 1;
    return;
  } else {
    // timeout reading ve.direct
    addLog(LOG_LEVEL_ERROR, "VICTR: Timeout reading VE.direct (MPPT)");
    return;
  }
}
