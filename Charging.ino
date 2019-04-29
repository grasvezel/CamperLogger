// Bepaal of de 12V accu's opgeladen moeten worden.
// Dat zou moeten als de 12V accu te leeg begint te raken
// maar dat kunnen we (nog) niet meten.
// Dus dan maar als we 24v 'over' hebben.

float chargeOnAt  = 27.40;
float chargeOffAt = 26.40;

void handleCharging() {
  if(readings.BMV_Vbatt >= chargeOnAt) {
    readings.Charger = 1;
  }
  if(readings.BMV_Vbatt <= chargeOffAt) {
    readings.Charger = 0;
  }
  digitalWrite(RELAY_PIN_1, readings.Charger);
}
