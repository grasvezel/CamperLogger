/*
 *  1-WIRE        GPIO 26
 *  GPS           GPIO 27
 *  VE.Direct 1   GPIO 32
 *  VE.Direct 2   GPIO 36
 *  LED           GPIO 13
 *  RELAY 1       GPIO 16
 *  RELAY 2       GPIO 21
 *  Tank sensor   GPIO 25
 */

// Partition scheme: Minimal SPIFFS (1.9MB APP with OTA)/190KB SPIFFS)

#include "FS.h"
#include "SPI.h"

#include "SPIFFS.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include <ctype.h>
#include <WiFi.h>
#include <ESP32WebServer.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include <OneWire.h>
#include <DallasTemperature.h>

void fileSystemCheck();
void addLog();
void WifiAPconfig();
void handleCharging();
String readVEdirectMPPT();

String getFileChecksum( String );

static float version              = 1.09;
static String verstr              = "Version 1.09"; // Make sure we can grep version from binary image

#define LOG_LEVEL_ERROR             1
#define LOG_LEVEL_INFO              2
#define LOG_LEVEL_DEBUG             3
#define LOG_LEVEL_DEBUG_MORE        4

byte logLevel                     = 4;                // not a #define, logLevel can be overridden from the server

#define FILE_SECURITY               "/security.dat"   // WiFi settings
#define FILE_SETTINGS               "/settings.dat"   // user settings
#define FILE_LOG                    "/log.txt"        // system log file
#define LOG_TO_FILE                 1

// TIME SETTINGS
// DST setting is stored in settings struct and is updated from the server
#define NTP_SERVER                  "pool.ntp.org"
#define TIME_ZONE                   60                // minutes ahead of GMT during winter.

// DATA LOGGING
#define DEFAULT_LOG_HOST            "bus.tarthorst.net"
#define LOG_INTERVAL                60                // In seconds

// Config AP setting
#define DEFAULT_PASSWORD            "poespuckje"

// PIN DEFINITIONS
#define PIN_STATUS_LED              2                 // LED on ESP32 dev board
#define PIN_EXT_LED                 13                // LED on PCB
#define WIFI_RECONNECT_INTERVAL     300               // seconds
#define GPS_PIN                     27                // serial
#define ONEWIRE_PIN                 26                // one wire input (temperature sensors)
#define RELAY_PIN_1                 16                // 12v charger control
#define RELAY_PIN_2                 21                // 
#define VE_DIRECT_PIN_1             32                // opto isolated input (RS-232 TTL)
#define VE_DIRECT_PIN_2             36                // opto isolated input (RS-232 TTL)
#define TANK_LEVEL_SENSOR_PIN       33                // Analog tank level sensor input

TaskHandle_t BackgroundTask;

struct SecurityStruct {
  char          WifiSSID[32];
  char          WifiKey[64];
  char          WifiSSID2[32];
  char          WifiKey2[64];
  char          WifiAPKey[64];
  char          Password[26];
  //its safe to extend this struct, up to 4096 bytes, default values in config are 0
} SecuritySettings;

struct SettingsStruct {
  byte          DST;
} Settings;

// DATA COLLECTION VARIABLES
String lastTelegramBMV = "";
String lastTelegramMPPT = "";

// Victron BMV vars
float Vbatt;
float Vaux;
float SOC;
float Ibatt;
float Pcharge;
float TTG;
float LDD;

// ve direct stuff
bool read_ve_direct_bmv  = 1;    // read BMV or skip it? Can be overridden from server.
bool read_ve_direct_mppt = 1;    // read MPPT or skip it? Can be overridden from server.
bool  charge12v = 0;

String request;
String tmp;

String Fcrc;
uint8_t ledChannelPin[16];
uint8_t chipid[6];
char chipMAC[12];
IPAddress apIP(192, 168, 4, 1);

unsigned long timerAPoff    = millis() + 10000L;
unsigned long timerLog      = millis() + LOG_INTERVAL * 1000L;
unsigned long nextWifiRetry = millis() + WIFI_RECONNECT_INTERVAL * 1000;

// please set temperature sensor addresses in TemperatureSenors.ino

OneWire oneWire(ONEWIRE_PIN);

ESP32WebServer WebServer(80);
HardwareSerial SerialGPS(1);     // GPS input (NMEA)
HardwareSerial SerialVE(2);      // VE direct connections (2 different rx pins, 1 uart)

void backgroundTasks(void * parameter) {
  for(;;) {
    // background tasks here
    vTaskDelay(10 / portTICK_PERIOD_MS); // 10ms delay to avoid WDT being triggered
  }
}

void setup() {
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_EXT_LED, OUTPUT);
  pinMode(GPS_PIN, INPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(VE_DIRECT_PIN_1, INPUT);
  pinMode(VE_DIRECT_PIN_2, INPUT);
  pinMode(TANK_LEVEL_SENSOR_PIN, INPUT);
  
  digitalWrite(PIN_EXT_LED, HIGH);
  delay(500);
  digitalWrite(PIN_EXT_LED, LOW);

  xTaskCreatePinnedToCore(
    backgroundTasks,  // Task function
    "Background",     // Name
    1000,             // Stack size
    NULL,             // Parameter
    1,                // priority
    &BackgroundTask,  // Task handle
    0);               // core

  Serial.begin(19200);
  SerialGPS.begin(9600, SERIAL_7E1, GPS_PIN, -1, false);

  addLog(LOG_LEVEL_INFO, "CORE : Version " + String(version) + " starting");

  for (byte x = 0; x < 16; x++)
    ledChannelPin[x] = -1;

  // get chip MAC
  esp_efuse_read_mac(chipid);
  sprintf(chipMAC, "%02x%02x%02x%02x%02x%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);

  fileSystemCheck();
  //list("/spiffs/", NULL);

  addLog(LOG_LEVEL_INFO, "FILE : Loading settings");
  LoadSettings();

  addLog(LOG_LEVEL_INFO, "CORE : DST setting: " + String(Settings.DST));
  
  WifiAPconfig();
  WifiConnect(3);
  if(WiFi.status() == WL_CONNECTED) {
    addLog(LOG_LEVEL_DEBUG, "WIFI : WiFi connected, disabling AP off timer");
    timerAPoff = 0;
  }
  delay(100);
  WebServerInit();
  
  // get the CRC of the current html file on SPIFFS
  Fcrc = getFileChecksum("/head.html");

  callHome();  // get settings and current software version from server
  addLog(LOG_LEVEL_INFO, "CORE : Setup done. Starting main loop");

}

void loop() {
  // check if AP should be turned on or off.
  updateAPstatus();
  // process incoming requests
  WebServer.handleClient();
  
  if (timerLog != 0 && timeOutReached(timerLog)) {
    timerLog = millis() + LOG_INTERVAL * 1000L;

    // periodic tasks
    request = "";
    addLog(LOG_LEVEL_INFO, "CORE : Performing periodic tasks");
    digitalWrite(PIN_EXT_LED, HIGH);

    // retry WiFi connection
    if(WiFi.status() != WL_CONNECTED && timeOutReached(nextWifiRetry)) {
      addLog(LOG_LEVEL_INFO, "WIFI : Not connected, trying to connect");
      WifiConnect(3);
      nextWifiRetry = millis() + WIFI_RECONNECT_INTERVAL * 1000;
    }

    callHome();

    // Victron BMV battery monitor
    // This needs to run even if we are not connected.
    // handleCharging() depends on these measurements.
    
    SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_1, -1, true);
    if(read_ve_direct_bmv) {
      request += readVEdirectBMV();
    }
    SerialVE.end();
    handleCharging();

    if (WiFi.status() == WL_CONNECTED) {
      // GPS
      request += handleGPS();
    
      // Victron MPPT charge controller
      SerialVE.begin(19200, SERIAL_8N1, VE_DIRECT_PIN_2, -1, true);
      if(read_ve_direct_mppt) {
        request += readVEdirectMPPT();
      }
      SerialVE.end();
    
      request += readTemperatureSensors();
      
      request += "&12vCharger=" + String(charge12v);

      request += readTankLevelSensor();

      addLog(LOG_LEVEL_INFO, "DATA : Uploading readings to server");
      String response = httpsGet("/update/", request);
      addLog(LOG_LEVEL_DEBUG, "DATA : Response from server: " + response);
    }
    digitalWrite(PIN_EXT_LED, LOW);
  }
}
