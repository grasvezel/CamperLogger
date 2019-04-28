String readTankLevelSensor() {
  int loops = 10;
  int currenttanklevel = 0;
  int tanklevel;
  
  for(int i=0;i<loops;i++) {
    currenttanklevel += analogRead(TANK_LEVEL_SENSOR_PIN);
  }
  currenttanklevel = currenttanklevel / loops;
  
  // tank levels
  // 724 11   -55 mm
  // 643 10   -70 mm
  // 565  9   -85 mm
  // 480  8   -100 mm
  // 442  7   -125 mm
  // 395  6   -140 mm
  // 351  5   -165 mm
  // 306  4   -185 mm
  // 238  3   -205 mm
  // 167  2   -225 mm
  // 20   1   -265 mm
  // 0    0   -290 mm

  tanklevel = 0;
  if(currenttanklevel > 10) {
     tanklevel = 1;
  }
  if(currenttanklevel > 94) {
     tanklevel = 2;
  }
  if(currenttanklevel > 202) {
     tanklevel = 3;
  }
  if(currenttanklevel > 272) {
     tanklevel = 4;
  }
  if(currenttanklevel > 329) {
     tanklevel = 5;
  }
  if(currenttanklevel > 373) {
     tanklevel = 6;
  }
  if(currenttanklevel > 419) {
     tanklevel = 7;
  }
  if(currenttanklevel > 461) {
     tanklevel = 8;
  }
  if(currenttanklevel > 523) {
     tanklevel = 9;
  }
  if(currenttanklevel > 604) {
     tanklevel = 10;
  }
  if(currenttanklevel > 684) {
     tanklevel = 11;
  }
  return("&Tnk=" + String(tanklevel) + "&Traw=" + String(currenttanklevel));
}

