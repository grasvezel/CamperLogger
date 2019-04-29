void runBackgroundTasks() {
    // These tasks will run in the background on CPU0

    // Read Victron BMV battary monitor
    if(read_ve_direct_bmv) {
      SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_1, -1, true);
      vTaskDelay(10 / portTICK_PERIOD_MS);
      GET_BMV = readVEdirectBMV();
      vTaskDelay(10 / portTICK_PERIOD_MS);
      SerialVE.end();
      handleCharging();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // Read Victron MPPT charge controller
    if(read_ve_direct_mppt) {
      SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_2, -1, true);
      vTaskDelay(10 / portTICK_PERIOD_MS);
      GET_MPPT = readVEdirectMPPT();
      vTaskDelay(10 / portTICK_PERIOD_MS);
      SerialVE.end();
    }

    handleGPS();
    readTemperatureSensors();
}
