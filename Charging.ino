// Bepaal of de 12V accu's opgeladen moeten worden.
// Dat zou moeten als de 12V accu te leeg begint te raken
// maar dat kunnen we (nog) niet meten.
// Dus dan maar als we 24v 'over' hebben.

float chargeOnAt  = 27.40;
float chargeOffAt = 26.40;

void handleCharging() {
  if(Vbatt >= chargeOnAt) {
    charge12v = 1;
  }
  if(Vbatt <= chargeOffAt) {
    charge12v = 0;
  }
  digitalWrite(RELAY_PIN_1, charge12v);
}

