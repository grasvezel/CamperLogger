void readTankLevelSensor() {
  int loops = 10;
  int currenttanklevel = 0;
  
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

  readings.Tank_level = 0;
  if(currenttanklevel > 10) {
     readings.Tank_level = 11;
  }
  if(currenttanklevel > 94) {
     readings.Tank_level = 28;
  }
  if(currenttanklevel > 202) {
     readings.Tank_level = 36;
  }
  if(currenttanklevel > 272) {
     readings.Tank_level = 45;
  }
  if(currenttanklevel > 329) {
     readings.Tank_level = 53;
  }
  if(currenttanklevel > 373) {
     readings.Tank_level = 64;
  }
  if(currenttanklevel > 419) {
     readings.Tank_level = 70;
  }
  if(currenttanklevel > 461) {
     readings.Tank_level = 77;
  }
  if(currenttanklevel > 523) {
     readings.Tank_level = 87;
  }
  if(currenttanklevel > 604) {
     readings.Tank_level = 94;
  }
  if(currenttanklevel > 684) {
     readings.Tank_level = 100;
  }
  return;
}
