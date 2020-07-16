void runBackgroundTasks() {
  // These tasks will run in the background.
  
  // pause background tasks during data upload.
  while(pause_background_tasks) {
    background_tasks_paused = 1;
    addLog(LOG_LEVEL_INFO, "BCKGR: Background tasks paused");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
//  while(WiFi.status() != WL_CONNECTED) {
//    addLog(LOG_LEVEL_INFO, "BCKGR: Not running background tasks, WiFi not connected.");
//    vTaskDelay(5000 / portTICK_PERIOD_MS);
//  }
  if(background_tasks_paused == 1) {
    addLog(LOG_LEVEL_INFO, "BCKGR: Resuming background tasks");
    background_tasks_paused = 0;
  }
  
  // In case something goes wrong, this buys us time to download a new image.
  if (firstbgrun) {
    vTaskDelay(10000 / portTICK_PERIOD_MS);
    addLog(LOG_LEVEL_INFO, "BCKGR: Delaying first background run");
    vTaskDelay(20000 / portTICK_PERIOD_MS);
    firstbgrun = 0;
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
  readWaterTankLevelSensor();
  readGasTankLevelSensor();

  // Now that we have read all devices, build inventory list
  inventory = "";
  if(readings.BMV_PID != "") {
    inventory += "VE.direct port 1:\n" + getVictronDeviceByPID(readings.BMV_PID)  + " (PID " + readings.BMV_PID  + ")\n";
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
    inventory += String(nr_of_temp_sensors) + " 1Wrire temperature sensor(s)\n";
  }
  if (inventory.length() == 0) {
    inventory = "No devices detected";
  }
  inventory_complete = 1;
}
