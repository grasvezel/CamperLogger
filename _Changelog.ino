/* 
 * Todo:  
 * 
 * Sometimes NTP does not work. Retry is after an hour. If NTP fails, it would be nice to retry sooner.
 * If NTP query fails, the current time is lost. Needs to be fixed.
 * It would be nice to make some stuff configurable from the server.
 * Settings need to be saved to SPIFFS
 * Add config for scanning 1-wire bus and autoconfigure the address(es) for the temp sensor(s).

  Version 1.17
  Bugfixes in the multitasking code. I had to bump the version number in order to force OTA updates.

  Version 1.11
  First attempt on doing some multitasking. Reading both VE.direct inputs is now moved to CPU0.
    
  Version 1.10
  All readings are now stored in a struct.
  /mppttelegram now shows last MPPT telegram
  
  Version 1.09
  Changed the way temperature sensors are handled. Addresses do not need to be known upfront,
  the oneWire bus is now scanned. Temperatures are reported in the order the sensors are
  found on the oneWire bus.

  Version 1.08
  More code cleanup. Started working on moving some tasks to the other cpu core.
  html head download now uses https
  added error handling when saving html head to SPIFFS
  FILE_SETTINGS and FILE_SECURITY are now actually used as filenames
    
  Version 1.07
  Code cleanup
  Translated comments into English
  VE.direct (BMV) timeout lowered to 4000ms (was 5000). Mainly for consistancy.
  
  Version 1.06
  Moved tank sensor to GPIO33 (was GPIO25)
  
  Versie 1.05
  Tank level sensor debugging
  
  Version 1.04
  Removed some leftover modbus code
  Added (analog) tank level sensor support

  Version 1.03
  Updated log messages

  Version 1.02
  readVEdirectBMV() now tries to find the beginning of the telegram and start from there.

  Versie 1.01
  No changes. Version number increased for OTA (Over-The-Air) software update testing.

  Version 1.00
  Removed SD support
  Changed PIN number definitions to match hardware version 2.
  Added second VE.direct input (for MPPT). UART is shared with the first one.
  Changed WiFi SSID to ACC_[id]
  Added VE.direct MPPT support (parser)
  Lowered GPS timeout to 2 seconds (was 5)
  Fixed bug that caused the internal IP address not to be logged
  nextWiFiretry now starts at WIFI_RECONNECT_INTERVAL * 1000 rather than 0
  Changed the order of the periodic tasks
  If we are not connected to WiFi, only the BMV is polled (to facilitate the charger logic). Other stuff is skipped.
  
  Version 0.38
  No changes, just OTA testing

  Versie 0.37
  Changed 12v charger logic thresholds to 27.4/26.4V
  
  Version 0.36
  Added timeout to second loop

  Version 0.35
  Added 5-second timeout reading VE.direct
  
  Version 0.34
  Added sanity check for Vbat (ve.direct).
  Changed 12v charger logic thresholds to 27.4/26.6
  
  Version 0.33
  Changed 12v charger logic thresholds to 27.6/26.8

*/
