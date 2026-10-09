#pragma once

// ======================== FIRMWARE IDENTITY ========================
// Read by the web installer tooling (cyd-web-installer) and Improv-Serial.
// FIRMWARE_VERSION is a #define so it can be pasted into string literals.
#define FIRMWARE_VERSION "3.8.0-dev"
// Frozen: Improv firmware name and installer manifest name. Renaming it turns
// the installer's "Update" (keeps WiFi) into "Install" (erases).
#define PROJECT_NAME "CYD_TFT_RetroClock"
#define PROJECT_REPO_URL "https://github.com/anthonyjclarke/CYD_TFT_RetroClock"
#define UPSTREAM_PROJECT "ESP8266 TFT LED Matrix Clock"
#define UPSTREAM_AUTHOR "cbm80amiga"
#define UPSTREAM_REPO_URL "https://github.com/cbm80amiga"

// ======================== WIFI SETUP ========================
#define AP_NAME "CYD_Clock_Setup"  // WiFiManager captive-portal hotspot

// Improv-Serial: web installer WiFi setup and "Update" detection. Always on;
// improvTick() must run at least every ~1 s (see src/network/improv_setup.h).
#define IMPROV_SETUP_ENABLED 1
#define IMPROV_DEVICE_PREFIX "RetroClock"
