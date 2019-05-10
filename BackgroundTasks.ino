void runBackgroundTasks() {
  // These tasks will run in the background on CPU0

  // In case something goes wrong, this buys us time to download a new image.
  if (!MPPT_present) {
    vTaskDelay(30000 / portTICK_PERIOD_MS);
  }
  // Read Victron BMV battary monitor
  if (read_ve_direct_bmv) {
    SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_1, -1, true);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    readVEdirect(DEVICE_BMV_B1);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    readVEdirect(DEVICE_BMV_B2);

    SerialVE.end();
    handleCharging();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  // Read Victron MPPT charge controller
  if (read_ve_direct_mppt) {
    SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_2, -1, true);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    readVEdirect(DEVICE_MPPT);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    SerialVE.end();
  }

  vTaskDelay(10 / portTICK_PERIOD_MS);
  readGPS();

  vTaskDelay(10 / portTICK_PERIOD_MS);

  readTemperatureSensors();

  vTaskDelay(10 / portTICK_PERIOD_MS);
  readTankLevelSensor();

  // Now that we have read all devices, the inventory list is complete.
  if(readings.BMV_PID != "") {
    inventory  = "VE.direct port 1:\n" + getVictronDeviceByPID(readings.BMV_PID)  + " (PID " + readings.BMV_PID  + ")\n";
    if(readings.BMV_serial != "") {
      inventory += "Serial number: " + readings.BMV_serial  + "\n";
    }
    inventory += "\n";
  }

  if(readings.MPPT_PID != "") {
    inventory += "VE.direct port 2:\n" + getVictronDeviceByPID(readings.MPPT_PID) + " (PID " + readings.MPPT_PID + ")\n";
    if(readings.MPPT_serial != "") {
      inventory += "Serial number: " + readings.MPPT_serial + "\n"; 
    }
    inventory += "\n";
  }

  if(GPS_present) {
    inventory += "GPS module detected";
    inventory += "\n\n";
  }

  nr_of_temp_sensors = 0;
  for (int i = 0; i < 10; i++) {
  vTaskDelay(10 / portTICK_PERIOD_MS);
  if (readings.temp[i] != -127) {
      nr_of_temp_sensors++;
    }
  }
  if(nr_of_temp_sensors > 0) {
    inventory += "Number of 1-Wire temperature sensors: " + String(nr_of_temp_sensors) + "\n";
  }
  if (inventory.length() == 0) {
    inventory = "No devices detected";
  }
  inventory_complete = 1;

}
