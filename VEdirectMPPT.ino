String readVEdirectMPPT() {
  float mppt_yield_total;       // H19
  float mppt_yield_today;       // H20
  int   mppt_maxpow_today;      // H21
  int   mppt_err;               // ERR
  int   mppt_state;             // CS
  float mppt_battery_voltage;   // V
  float mppt_battery_current;   // I
  float mppt_panel_voltage;     // VPV
  int   mppt_panel_power;       // PPV

  String thisTelegram;

  String text = ""; // http GET data
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
          mppt_battery_voltage = line.substring(2).toFloat() / 1000;
          if (mppt_battery_voltage > 3) {
            valuesread++;
            text += "&MUb=" + String(mppt_battery_voltage);
          }
        }

        // Battery current
        if (line.startsWith("I\t")) {
          mppt_battery_current = line.substring(2).toFloat() / 1000;
          valuesread++;
          text += "&MIb=" + String(mppt_battery_current);
        }

        // PV voltage
        if (line.startsWith("VPV\t")) {
          mppt_panel_voltage = line.substring(4).toFloat() / 1000;
          valuesread++;
          text += "&MUpv=" + String(mppt_panel_voltage);
        }

        // PV power
        if (line.startsWith("PPV\t")) {
          valuesread++;
          mppt_panel_power = line.substring(4).toInt();
          text += "&MPpv=" + String(mppt_panel_power);
        }

        // PV Max power today
        if (line.startsWith("H21\t")) {
          valuesread++;
          mppt_maxpow_today = line.substring(4).toInt();
          text += "&MPmxt=" + String(mppt_maxpow_today);
        }

        // PV yield today
        if (line.startsWith("H20\t")) {
          valuesread++;
          mppt_yield_today = line.substring(4).toFloat() / 100;
          text += "&MYt=" + String(mppt_yield_today);
        }

        // PV yield total
        if (line.startsWith("H19\t")) {
          valuesread++;
          mppt_yield_total = line.substring(4).toFloat() / 100;
          text += "&MYtot=" + String(mppt_yield_total);
        }

        // MPPT error number
        if (line.startsWith("ERR\t")) {
          valuesread++;
          mppt_err = line.substring(4).toInt();
          text += "&Merr=" + String(mppt_err);
        }

        // MPPT charge state
        if (line.startsWith("CS\t")) {
          valuesread++;
          mppt_state = line.substring(3).toInt();
          text += "&Mstate=" + String(mppt_state);
        }

        line = "";
      } else {
        line += character;
      }
    }
    digitalWrite(PIN_EXT_LED, LOW);
  }

  if (valuesread == expectedvalues) {
    lastTelegramMPPT  = "Accuspanning      : " + String(mppt_battery_voltage) + "V\n";
    lastTelegramMPPT += "Accustroom        : " + String(mppt_battery_current) + "A\n";
    lastTelegramMPPT += "Paneelspanning    : " + String(mppt_panel_voltage) + "V\n";
    lastTelegramMPPT += "Paneelvermogen    : " + String(mppt_panel_power) + "W\n";
    lastTelegramMPPT += "Pmax vandag       : " + String(mppt_maxpow_today) + "W\n";
    lastTelegramMPPT += "Opbrengst vandaag : " + String(mppt_yield_today) + "kWh\n";
    lastTelegramMPPT += "Opbrengst totaal  : " + String(mppt_yield_total) + "kWh\n";
    lastTelegramMPPT += "State             : " + String(mppt_state) + "\n";
    lastTelegramMPPT += "Error             : " + String(mppt_err) + "\n";
    return (text);
  } else {
    // timeout reading ve.direct
    addLog(LOG_LEVEL_ERROR, "VICTR: Timeout reading VE.direct (MPPT)");
    return ("");
  }
}
