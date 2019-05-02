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
#include <base64.h>

//------------------------------------------------------------------------------
// Settings
// The idea is that this struct is going to be saved on SPIFFS in the future.
// Maybe configurable through the web interface. In any case credentials should
// not be in the code. This is a tepmorary solution
//------------------------------------------------------------------------------

struct SettingsStruct {
  byte          DST;
  bool          upload_get      = 1;
  bool          upload_influx   = 1;
  String        influx_host     = "bus.tarthorst.net";
  int           influx_port     = 8086;
  String        influx_db       = "test";
  String        influx_mn       = "camper";                      // the name of the measurement
  String        influx_user     = "testuser";
  String        influx_pass     = "testpass";
} Settings;


//void fileSystemCheck();
//void addLog();
//void WifiAPconfig();
//void handleCharging();

String getFileChecksum( String );

static float version              = 1.76;
static String verstr              = "Version 1.76";   //Make sure we can grep version from binary image

#define LOG_LEVEL_ERROR             1
#define LOG_LEVEL_INFO              2
#define LOG_LEVEL_DEBUG             3
#define LOG_LEVEL_DEBUG_MORE        4

byte logLevel                     = 4;                // not a #define, logLevel can be overridden from the server

#define FILE_SECURITY               "/security.dat"   // WiFi settings
#define FILE_SETTINGS               "/settings.dat"   // user settings

// TIME SETTINGS
// DST setting is stored in settings struct and is updated from the server
#define NTP_SERVER                  "pool.ntp.org"
#define TIME_ZONE                   60                // minutes ahead of GMT during winter.

// DATA LOGGING
#define DEFAULT_LOG_HOST            "bus.tarthorst.net"
#define LOG_INTERVAL                60                // In seconds

// Geohash
#define GEOHASH_PRECISION           8

// AP password
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

#define DEVICE_BMV_B1               1                 // Block one of the BMV output
#define DEVICE_BMV_B2               2                 // Block two of the BMV output
#define DEVICE_MPPT                 3                 // MPPT output has only one block

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

struct readingsStruct {
  // Temperature sensors
  float temp[10];   // max 10 temperature sensors (deg C)
  
  // BMV vars
  float  BMV_Vbatt;   // BMV battery voltage (V)
  float  BMV_Vaux;    // BMV auxilary voltage (V)
  float  BMV_SOC;     // BMV State Of Charge (%)
  float  BMV_Ibatt ;  // BMV battery current (A)
  int    BMV_Pcharge; // BMV charge power (W)
  int    BMV_TTG;     // BMV Time To Go (minutes)
  float  BMV_LDD;     // BMV Last Discharge Depth (Ah)
  bool   BMV_B1_ok;   // BMV checksum on block one OK
  bool   BMV_B2_ok;   // BMV checksum on block two OK
  String BMV_PID;     // BMV Product ID
  String BMV_serial;  // BMV serial number

  // MPPT vars
  float  MPPT_ytot;   // MPPT yield total (kWh)    H19
  float  MPPT_yday;   // MPPT yield today (kWh)    H20
  int    MPPT_Pmax;   // MPPT max power today (W)  H21
  int    MPPT_err;    // MPPT error number         ERR
  int    MPPT_state;  // MPPT state                CS
  float  MPPT_Vbatt;  // MPPT output voltage (V)   V
  float  MPPT_Ibatt;  // MPPT output current (A)   I
  float  MPPT_Vpv;    // MPPT input voltage (V)    VPV
  int    MPPT_Ppv;    // MPPT input power (W)      PPV
  bool   MPPT_ok;     // MPPT checksum on last block OK
  String MPPT_PID;    // MPPT Product ID
  String MPPT_serial; // MPPT serial number

  // GPS readings
  String GPS_fix;    // GPS status (active/timeout/void)
  String GPS_date;   // GPS date DDMMYY
  String GPS_time;   // GPS time HHMMSS (in UTC!)
  String GPS_lat;    // GPS latitude (0...90 N or S)
  String GPS_lon;    // GPS longitude (0...180 E or W)
  String GPS_speed;  // GPS speed in km/h
  String GPS_heading;// GPS heading (in deg, 0 if not moving)
  String GPS_geohash;// Geohash

  // water tank
  int   Tank_level;  // Water tank level (%)

  // 12v charger
  int   Charger;  // 12v charger on
} readings;

// DATA COLLECTION VARIABLES
String lastBlockBMV = "";
String lastBlockMPPT = "";
String inventory = "";
bool   inventory_complete = 0;
int    nr_of_temp_sensors = 0;
bool   inventory_requested = 0;

// Since we are going to multitask, we want to avoid posting
// data to the server while the values are being read.
// FIXME - NOT YET IMPLEMENTED
bool pause_readings  = 0;
bool readings_paused = 0;

// We are only going to upload readings from connected devices.
// These values will be set to 1 after the first valid reading
// from the respective device is received.
bool BMV_present = 0;
bool MPPT_present = 0;
bool GPS_present = 0;

// toggle different data sources. Do we need this?
bool read_ve_direct_bmv  = 1;    // read BMV or skip it?
bool read_ve_direct_mppt = 1;    // read MPPT or skip it?
bool read_temp           = 1;    // read temperature sensors or skip it?
bool read_gps            = 1;    // read GPS data or skip it?
bool read_tank_level     = 1;    // read tank level sensor or skip it?

String tmp;

String Fcrc;
uint8_t ledChannelPin[16];
uint8_t chipid[6];
char chipMAC[12];
IPAddress apIP(192, 168, 4, 1);

unsigned long timerAPoff    = millis() + 10000L;
unsigned long timerLog      = millis() + LOG_INTERVAL * 1000L;
unsigned long nextWifiRetry = millis() + WIFI_RECONNECT_INTERVAL * 1000;

// prototypes with default ports for http and https
String httpsGet(String path, String query, int port=443);
String httpGet(String path, String query, int port=80);

OneWire oneWire(ONEWIRE_PIN);

ESP32WebServer WebServer(80);

// Serial ports
// UART 0 is used for the console. Because the ESP32 has 3 UARTs and we need 3,
// HardwareSerial(2) is switched to the according pin if we want to read BMV or MPPT.
HardwareSerial SerialGPS(1);     // GPS input (NMEA)
HardwareSerial SerialVE(2);      // VE direct connections

// Background tasks. Not in use atm.
void backgroundTasks(void * parameter) {
  for(;;) {
    runBackgroundTasks();
    // 10ms delay to avoid WDT being triggered
    vTaskDelay(10 / portTICK_PERIOD_MS);
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
    4000,             // Stack size
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
    addLog(LOG_LEVEL_INFO, "CORE : Performing periodic tasks");
    digitalWrite(PIN_EXT_LED, HIGH);

    // retry WiFi connection
    if(WiFi.status() != WL_CONNECTED && timeOutReached(nextWifiRetry)) {
      addLog(LOG_LEVEL_INFO, "WIFI : Not connected, trying to connect");
      WifiConnect(3);
      nextWifiRetry = millis() + WIFI_RECONNECT_INTERVAL * 1000;
    }

    callHome();
    uploadGetData();
    uploadInfluxData();
    digitalWrite(PIN_EXT_LED, LOW);
  }
}
