#pragma once

#include <Arduino.h>

// Leveled debug macros, used by src/network/improv_setup.cpp. The main sketch
// still uses its own DEBUG(x) macro.
//   1 = Error · 2 = Warn · 3 = Info (default) · 4 = Verbose
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 3
#endif

// clang-format off
#define DBG_ERROR(fmt, ...)   do { if (DEBUG_LEVEL >= 1) Serial.printf("[ERR ] " fmt "\n", ##__VA_ARGS__); } while(0)
#define DBG_WARN(fmt, ...)    do { if (DEBUG_LEVEL >= 2) Serial.printf("[WARN] " fmt "\n", ##__VA_ARGS__); } while(0)
#define DBG_INFO(fmt, ...)    do { if (DEBUG_LEVEL >= 3) Serial.printf("[INFO] " fmt "\n", ##__VA_ARGS__); } while(0)
#define DBG_VERBOSE(fmt, ...) do { if (DEBUG_LEVEL >= 4) Serial.printf("[VERB] " fmt "\n", ##__VA_ARGS__); } while(0)
// clang-format on
