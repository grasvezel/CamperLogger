#Camper-Logger

This is the firmware for an ESP32-based data logger for campers.

It can be used to gather some data (see below) and upload it to some form of data storage.
You can either use an external host (if you have connectivity in your camper), or a local
host (like a Raspberry Pi) if you don't. The logger itself does not store any data!

## Features

- Read GPS coordinates, speed, heading and time
- Read battery status using a Victron BMV battery monitor
- Read solar statistics from a Victron MPPT charge regulator
- Measure temperatures using Dallas 1-Wire sensors
- Measure the level in your fresh water tank (using a resistive sensor)
- Switch stuff on/off using relays (I use it to switch my 24V to 12V battery charger)
- Over the air software updates (OTA)
- Periodically upload this info to the server

- Data upload is done over https

If you are interested in the hardware design and/or the PCB, send me a message.

