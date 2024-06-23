# ESP32 AWS Provisioning

### This is a WIP project. Upcoming additions are AWS onboard & communication (via MQTT), I2C read/write.

This repository contains code (developed using ESP-IDF) to:

1. Onboard an ESP32 to WiFi.

## Functionality

### Initial Connection

- Tries to connect to SSID/PASS provided in `ssid.txt` and `pass.txt` files (stored on SPIFFS). If unable to find these files:

### Access Point Mode

1. Enters AP mode.
2. Serves an HTML page for the user to input SSID and password.
3. Writes the provided SSID and password to SPIFFS (`/ssid.txt`, `/pass.txt`).
4. Reboots the ESP32.
5. Connects to the provided WiFi configuration.
6. Serves a WiFi connected page.

## Usage

### Step-by-Step Process

1. **Initial Attempt:**
    - The ESP32 will try to connect using credentials in `ssid.txt` and `pass.txt`.

2. **If Initial Attempt Fails:**
    - The ESP32 will enter AP mode and serve a configuration page at `<HOST_PREFIX>.local`.
    - The user can input the SSID and password on this page.

3. **Post-Configuration:**
    - The ESP32 will save the new credentials to SPIFFS and reset.
    - On reboot, it will connect to the configured WiFi and serve a connected page at `<HOST_PREFIX>wifi`.

## Getting Started
1. For now, change required config var in `/main/wifi_connect.c`

## Additional
1. `build_spiffs.ps1` is a powershell script to build and flash a spiffs image onto your esp32. Place files you want to flash onto folder `/data`

### TODO
1. add master config.h for changing definitions
2. aws onboard (wip)


Jatan Pandya
6/23/2024