void runBackgroundTasks() {
    // These tasks will run in the background on CPU0
    // Read Victron BMV battary monitor
    if(read_ve_direct_bmv) {
      SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_1, -1, true);
      vTaskDelay(10 / portTICK_PERIOD_MS);
      BMV_readings_valid = 0;
      GET_BMV = readVEdirectBMV();
      BMV_readings_valid = 1;
      vTaskDelay(10 / portTICK_PERIOD_MS);
      SerialVE.end();
      handleCharging();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    // Read Victron MPPT charge controller
    if(read_ve_direct_mppt) {
      SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_2, -1, true);
      vTaskDelay(10 / portTICK_PERIOD_MS);
      MPPT_readings_valid = 0;
      GET_MPPT = readVEdirectMPPT();
      MPPT_readings_valid = 1;
      vTaskDelay(10 / portTICK_PERIOD_MS);
      SerialVE.end();
    }
}
