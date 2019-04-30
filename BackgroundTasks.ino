void runBackgroundTasks() {
    // These tasks will run in the background on CPU0

    // In case something goes wrong, this buys us time to download a new image.
    if(!MPPT_present) {
      vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
    // Read Victron BMV battary monitor
    if(read_ve_direct_bmv) {
      SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_1, -1, true);
      vTaskDelay(10 / portTICK_PERIOD_MS);
      readVEdirectBMV();
      vTaskDelay(10 / portTICK_PERIOD_MS);
      SerialVE.end();
      handleCharging();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // Read Victron MPPT charge controller
    if(read_ve_direct_mppt) {
      SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_2, -1, true);
      vTaskDelay(10 / portTICK_PERIOD_MS);
      readVEdirectMPPT();
      vTaskDelay(10 / portTICK_PERIOD_MS);
      SerialVE.end();
    }

    readGPS();
    readTemperatureSensors();
    readTankLevelSensor();
    
}
