// Determine if 12v battery should be charged
// The charger is switched on by relay 1 if the main battery voltage (as
// measured by the BMV) rises above chargeOnAt and off again if the voltage
// drops below chargeOffAt. suggested values are 27.40 and 26.40 respectively.

float chargeOnAt  = 26.50;
float chargeOffAt = 26.00;

void handleCharging() {
}
