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
      WebServer.handleClient();
      digitalWrite(PIN_EXT_LED, HIGH);
      character = SerialVE.read();
      if (character == '\n') {
        // regel einde
        if (line.startsWith("V\t")) {
          Vbatt = line.substring(2).toFloat() / 1000;
          if(Vbatt > 3) {
            valuesread++;
            Serial.println("Accuspanning        : " + String(Vbatt) + "V");
            text += "&Ub=" + String(Vbatt);
          }
        }
        if (line.startsWith("VS\t") || line.startsWith("VM\t")) {
          Vaux = line.substring(3).toFloat() / 1000;
          if(Vaux > 1.5) {
            valuesread++;
            Serial.println("12V spanning        : " + String(Vaux) + "V");
            text += "&Um=" + String(Vaux);
          }
        }
        if (line.startsWith("SOC\t")) {
          valuesread++;
          SOC = line.substring(4).toFloat() / 10;
          Serial.println("State of Charge     : " + String(SOC) + "%");
          text += "&SOC=" + String(SOC);
        }
        if (line.startsWith("I\t")) {
          valuesread++;
          Ibatt = line.substring(2).toFloat() / 1000;
          Serial.println("Accustroom          : " + String(Ibatt) + "A");
          text += "&Ib=" + String(Ibatt);
        }
        if (line.startsWith("TTG\t")) {
          valuesread++;
          TTG = line.substring(4).toFloat();
          Serial.println("Time to go          : " + String(TTG) + "min");
          text += "&TTG=" + String(TTG);
        }
        if (line.startsWith("H2\t")) {
          valuesread++;
          LDD = line.substring(3).toFloat() / 1000;
          Serial.println("Last Discharge Depth: " + String(LDD ) + "Ah");
          text += "&LDD=" + String(LDD);
        }
        if (line.startsWith("P\t")) {
          valuesread++;
          Pcharge = line.substring(2).toFloat();
          Serial.println("Laadvermogen        : " + String(Pcharge) + "W");
          text += "&Pb=" + String(int(Pcharge));
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
    lastTelegramBMV  = "Accuspanning      : " + String(Vbatt) + "V\n";
    lastTelegramBMV += "Middenspanning    : " + String(Vaux) + "V\n";
    lastTelegramBMV += "Laadstatus (SOC)  : " + String(SOC) + "%\n";
    lastTelegramBMV += "Laadstroom        : " + String(Ibatt) + "A\n";
    lastTelegramBMV += "Laadvermogen      : " + String(Pcharge) + "W\n";
    lastTelegramBMV += "Time to Go        : " + String(TTG) + "\n";
    lastTelegramBMV += "Laatste ontlading : " + String(LDD) + "Ah\n";
    return(text);
  } else {
    // timeout reading ve.direct
    addLog(LOG_LEVEL_ERROR, "VICTR: Timeout reading VE.direct (BMV)");
    return("");
  }
}
