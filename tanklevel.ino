String readTankLevelSensor() {
  int loops = 10;
  int currenttanklevel = 0;
  int tanklevel;
  
  for(int i=0;i<loops;i++) {
    currenttanklevel += analogRead(TANK_LEVEL_SENSOR_PIN);
  }
  currenttanklevel = currenttanklevel / loops;
  
  // tank levels
  // 724 11   -55 mm    235   100%
  // 643 10   -70 mm    220    94%
  // 565  9   -85 mm    205    87%
  // 480  8   -100 mm   180    77%
  // 442  7   -125 mm   165    70%
  // 395  6   -140 mm   150    64%
  // 351  5   -165 mm   125    53%
  // 306  4   -185 mm   105    45%
  // 238  3   -205 mm    85    36%
  // 167  2   -225 mm    65    28%
  // 20   1   -265 mm    25    11%
  // 0    0   -290 mm     0     0%

  tanklevel = 0;
  if(currenttanklevel > 10) {
     tanklevel = 11;
  }
  if(currenttanklevel > 94) {
     tanklevel = 28;
  }
  if(currenttanklevel > 202) {
     tanklevel = 36;
  }
  if(currenttanklevel > 272) {
     tanklevel = 45;
  }
  if(currenttanklevel > 329) {
     tanklevel = 53;
  }
  if(currenttanklevel > 373) {
     tanklevel = 64;
  }
  if(currenttanklevel > 419) {
     tanklevel = 70;
  }
  if(currenttanklevel > 461) {
     tanklevel = 77;
  }
  if(currenttanklevel > 523) {
     tanklevel = 87;
  }
  if(currenttanklevel > 604) {
     tanklevel = 94;
  }
  if(currenttanklevel > 684) {
     tanklevel = 100;
  }
  return("&Tnk=" + String(tanklevel) + "&Traw=" + String(currenttanklevel));
}
