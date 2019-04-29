String readVEdirectBMV() {
  String text = ""; // http GET data
  int valuesread = 0;
  char character;
  char prev_character;
  String line = "";
  // The BMV sends *half* a telegram every second, so we need
  // twice the timeout compared to the MPPT.
  unsigned long vedirect_timeout = millis() + 4000L;

  // Try to find the beginning of the telegram.
  while (SerialVE.available() > 0 && !timeOutReached(vedirect_timeout)) {
    character = SerialVE.read();
    if(character == '\n' && prev_character == '\n') {
      Serial.println("Begin telegram");
      break;
    }
    prev_character = character;
  }

  while (valuesread < 7 && !timeOutReached(vedirect_timeout)) {
    while (SerialVE.available() > 0 && !timeOutReached(vedirect_timeout)) {
      digitalWrite(PIN_EXT_LED, HIGH);
      character = SerialVE.read();
      if (character == '\n') {
        // regel einde
        if (line.startsWith("V\t")) {
          readings.BMV_Vbatt = line.substring(2).toFloat() / 1000;
          if(readings.BMV_Vbatt > 3) {
            valuesread++;
            Serial.println("Accuspanning        : " + String(readings.BMV_Vbatt) + "V");
            text += "&Ub=" + String(readings.BMV_Vbatt);
          }
        }
        if (line.startsWith("VS\t") || line.startsWith("VM\t")) {
          readings.BMV_Vaux = line.substring(3).toFloat() / 1000;
          if(readings.BMV_Vaux > 1.5) {
            valuesread++;
            Serial.println("12V spanning        : " + String(readings.BMV_Vaux) + "V");
            text += "&Um=" + String(readings.BMV_Vaux);
          }
        }
        if (line.startsWith("SOC\t")) {
          valuesread++;
          readings.BMV_SOC = line.substring(4).toFloat() / 10;
          Serial.println("State of Charge     : " + String(readings.BMV_SOC) + "%");
          text += "&SOC=" + String(readings.BMV_SOC);
        }
        if (line.startsWith("I\t")) {
          valuesread++;
          readings.BMV_Ibatt = line.substring(2).toFloat() / 1000;
          Serial.println("Accustroom          : " + String(readings.BMV_Ibatt) + "A");
          text += "&Ib=" + String(readings.BMV_Ibatt);
        }
        if (line.startsWith("TTG\t")) {
          valuesread++;
          readings.BMV_TTG = line.substring(4).toFloat();
          Serial.println("Time to go          : " + String(readings.BMV_TTG) + "min");
          text += "&TTG=" + String(readings.BMV_TTG);
        }
        if (line.startsWith("H2\t")) {
          valuesread++;
          readings.BMV_LDD = line.substring(3).toFloat() / 1000;
          Serial.println("Last Discharge Depth: " + String(readings.BMV_LDD ) + "Ah");
          text += "&LDD=" + String(readings.BMV_LDD);
        }
        if (line.startsWith("P\t")) {
          valuesread++;
          readings.BMV_Pcharge = line.substring(2).toFloat();
          Serial.println("Laadvermogen        : " + String(readings.BMV_Pcharge) + "W");
          text += "&Pb=" + String(int(readings.BMV_Pcharge));
        }
        Serial.println(line);
        line = "";
      } else {
        line += character;
      }
    }
    digitalWrite(PIN_EXT_LED, LOW);
  }
  if(valuesread == 7) {
    lastTelegramBMV  = "Accuspanning      : " + String(readings.BMV_Vbatt) + "V\n";
    lastTelegramBMV += "Middenspanning    : " + String(readings.BMV_Vaux) + "V\n";
    lastTelegramBMV += "Laadstatus (SOC)  : " + String(readings.BMV_SOC) + "%\n";
    lastTelegramBMV += "Laadstroom        : " + String(readings.BMV_Ibatt) + "A\n";
    lastTelegramBMV += "Laadvermogen      : " + String(readings.BMV_Pcharge) + "W\n";
    lastTelegramBMV += "Time to Go        : " + String(readings.BMV_TTG) + "\n";
    lastTelegramBMV += "Laatste ontlading : " + String(readings.BMV_LDD) + "Ah\n";
    BMV_present = 1;
    return(text);
  } else {
    // timeout reading ve.direct
    addLog(LOG_LEVEL_ERROR, "VICTR: Timeout reading VE.direct (BMV)");
    return("&BMV=timeout&valuesread=" + String(valuesread));
  }
}
