# Project: ESP32 CYD TFT Matrix Clock

## Overview
Retro LED matrix clock simulator running on the ESP32 Cheap Yellow Display. Simulates a 4x2 MAX7219 LED matrix (32×16 pixels) with authentic LED rendering on a 320×240 ILI9341 TFT display. Features WiFiManager configuration, NTP time sync with 87 global timezones, optional I2C environmental sensors (BME280/SHT3X/HTU21D), and a modern responsive web interface with live display mirroring. Three auto-cycling display modes show time with temperature, large time display, or time with date in multiple formats. Based on original ESP8266 project by @cbm80amiga (Pawel A.).

## Hardware
- **MCU**: ESP32-2432S028R (Cheap Yellow Display - CYD)
  - Dual-core ESP32 @ 240 MHz
  - 4MB Flash
  - Built-in 2.8" ILI9341 TFT (320×240, RGB565)
  - XPT2046 resistive touchscreen (not used)
  - RGB LED (active-low)
  - SD card slot (not used)
- **Peripherals**:
  - Optional I2C environmental sensor (BME280, SHT3X, or HTU21D)
  - BOOT button for WiFi reset
- **Power**: USB 5V / 1A minimum (stable power critical for WiFi)

## Build Environment
- **Framework**: Arduino for ESP32
- **Platform**: espressif32
- **IDE**: VSCode with PlatformIO extension
- **Upload**: USB (esptool) or OTA (espota with password auth)
- **Monitor**: 115200 baud
- **Key Libraries**:
  - TFT_eSPI v2.5.43 (display driver)
  - WiFiManager (tzapu fork for ESP32)
  - Adafruit sensor libraries (BME280 v2.2.4, SHT31 v2.2.2, HTU21DF v1.0.5)
  - ArduinoOTA (OTA updates with CYD_OTA_2024 password)

## Project Structure
```
CYD_TFT_RetroClock/
├── src/
│   └── cyd_tft_clock.cpp      # Main application (1800+ lines)
│                              # - Display rendering engine
│                              # - Web server with REST API
│                              # - WiFi/NTP/sensor management
├── include/
│   ├── User_Setup.h           # TFT_eSPI pin configuration (not used, see platformio.ini)
│   ├── fonts.h                # LED matrix bitmap fonts (3x7, 5x8, 7x16)
│   └── timezones.h            # 88 POSIX timezone strings with DST rules
├── platformio.ini             # Build config with TFT pins as build flags
├── .vscode/
│   ├── tasks.json             # Custom PlatformIO tasks (OTA upload, build-only)
│   └── keybindings.json       # Cmd+U (OTA), Cmd+Shift+B (build)
├── images/                    # Reference images and mode screenshots
├── README.md                  # Comprehensive user documentation
├── CHANGELOG.md               # Version history (current: v3.6)
└── CLAUDE.md                  # This file (development reference)
```

## Pin Mapping

### TFT Display (HSPI)
| Function | GPIO | Notes |
|----------|------|-------|
| MOSI | 13 | SPI data out (40 MHz) |
| MISO | 12 | SPI data in (not used in write-only) |
| SCLK | 14 | SPI clock |
| CS | 15 | Chip select |
| DC | 2 | Data/Command select |
| RST | -1 | Tied to EN (no manual reset) |
| Backlight | 21 | HIGH = on, LOW = off |

### I2C Sensor (CN1 Extended Connector)
| Function | GPIO | Notes |
|----------|------|-------|
| SDA | 27 | I2C data |
| SCL | 22 | I2C clock |
| VCC | 3.3V | **Must use 3.3V, NOT 5V** |
| GND | GND | Ground |

### RGB LED (Active LOW)
| Function | GPIO | Notes |
|----------|------|-------|
| Red | 4 | LOW = on, HIGH = off |
| Green | 16 | LOW = on, HIGH = off |
| Blue | 17 | LOW = on, HIGH = off |

### Control Inputs
| Function | GPIO | Notes |
|----------|------|-------|
| BOOT Button | 0 | Active LOW, hold 3s during power-up to reset WiFi |

### Touchscreen (VSPI - Not Used)
| Function | GPIO | Notes |
|----------|------|-------|
| T_IRQ | 36 | Not used in this project |
| T_DIN | 32 | Not used in this project |
| T_OUT | 39 | Not used in this project |
| T_CLK | 25 | Not used in this project |
| T_CS | 33 | Not used in this project |

## Configuration

### Sensor Selection (src/cyd_tft_clock.cpp:82-85)
Uncomment ONE sensor type before compilation:
```cpp
// #define USE_BME280     // Temp/Humidity/Pressure (0x76 or 0x77)
// #define USE_SHT3X      // Temp/Humidity (0x44 or 0x45)
#define USE_HTU21D        // Temp/Humidity (0x40) - CURRENTLY ACTIVE
```

### Display Configuration (src/cyd_tft_clock.cpp:126-166)
- LED matrix dimensions: 32×16 pixels (8 simulated 8×8 matrices)
- LED size: 9px default (adjustable 4-12px via web UI)
- LED spacing: 1px default (adjustable 0-3px via web UI)
- Display rotation: 3 (180° flipped, toggleable via web UI)
- Color presets: Red, Green, Blue, Yellow, Cyan, Magenta, White, Orange

### Timing Constants (src/cyd_tft_clock.cpp:169-172)
- Sensor update: 60000ms (60s)
- NTP sync: 3600000ms (1 hour)
- Status print: 60000ms (60s)
- Mode switch: 5s default (1-60s via web UI)

### Debug Output (src/cyd_tft_clock.cpp:174-187)
- `DEBUG_ENABLED 1` enables Serial output at 115200 baud
- Detailed WiFi diagnostics (SSID, IP, RSSI, Gateway, DNS)
- Per-second time/temp output on Serial
- Settings changes logged only when modified

### OTA Configuration (platformio.ini:16-26)
- Port: 3232
- Password: `CYD_OTA_2024` (must match in code at src/cyd_tft_clock.cpp:1965)
- Protocol: espota
- Safety: `upload_port` commented out to prevent accidental OTA when USB connected
- Usage: `pio run -t upload --upload-port 192.168.1.xxx`

### Web Interface Settings (Runtime Configurable)
- Temperature units: °C / °F
- Time format: 12h / 24h
- Leading zero: On / Off (hours < 10)
- Date format: 5 options (DD/MM/YY, MM/DD/YY, YYYY-MM-DD, DD.MM.YYYY, MM.DD.YYYY)
- Timezone: 87 global options (default: Sydney, Australia - index 0)
- Display style: Blocks / Realistic LEDs
- LED color: 8 presets
- Surround color: 8 presets + "Match LED"
- LED size: 4-12px (affects seconds visibility)
- LED spacing: 0-3px
- Mode interval: 1-60s
- Display rotation: Normal / 180° flipped

## Current State

**Version**: 3.6 (2026-01-08)
**Status**: Production-ready, stable

### Recent Changes (v3.6 - 2026-01-08)
- Date format selector with 5 common formats (DD/MM/YY, MM/DD/YY, YYYY-MM-DD, DD.MM.YYYY, MM.DD.YYYY)
- Display rotation flip control (normal/180°)
- OTA upload safety improvements (prevents accidental USB/OTA conflicts)
- VS Code tasks and keybindings for PlatformIO workflow
- Automatic LED size adjustment based on date format (9px for short, 8px for long)

### Working Features
✅ WiFiManager captive portal with 3 reset methods
✅ NTP time sync with 87 timezones and DST support
✅ Three auto-cycling display modes (configurable 1-60s interval)
✅ Multi-sensor support (BME280/SHT3X/HTU21D)
✅ Web interface with live TFT display mirror (500ms refresh)
✅ RGB LED status indicators (startup and runtime)
✅ OTA firmware updates (password-protected)
✅ Serial diagnostics at 115200 baud
✅ Network monitoring with auto-reconnect (checks every 10s)
✅ Comprehensive timezone support (87 global cities)

### Performance Characteristics
- Main loop: 1ms delay (fast, responsive)
- WiFi check: Every loop iteration (~every 10s when combined with other tasks)
- WiFi auto-reconnect: 5s timeout, then ESP.restart() if failed
- Display refresh: On change only (FAST_REFRESH enabled, pixel-level dirty tracking)
- Web interface updates: Time API polled every 1000ms, Display canvas every 500ms
- Memory usage: Monitor via Serial (free heap printed every 60s)

## Architecture Notes

### Display Rendering Engine
- **Virtual LED Matrix**: 64-byte buffer (`byte scr[64]`) represents 32×16 pixel matrix (4 matrices wide × 2 matrices tall)
- **Bitmap Fonts**: Custom fonts in PROGMEM (fonts.h) with column-based encoding
  - `digits7x16[]` - Large 7×16 digits for Mode 1
  - `digits5x16rn[]` - Alternate 5×16 digit set
  - `font3x7[]` - Small 3×7 font for seconds, date, temp/humidity
- **Smart Refresh**: `FAST_REFRESH` flag enables pixel-level dirty tracking (only redraws changed pixels via static lastScr[] buffer)
- **Two Render Modes**:
  - Default (0): Solid rectangular blocks
  - Realistic (1): Circular LEDs with dim "off" state and authentic 4px row gap between top/bottom matrices
- **Color Dimming**: `dimRGB565()` preserves hue while reducing brightness for "off" LEDs (divides RGB components by factor+1)
- **Centering Logic**: Calculates offsets to center LED matrix on 320×240 display dynamically based on LED size
- **TFT Functions**:
  - `drawLEDPixel()` - Renders single virtual pixel as LED on TFT
  - `refreshAll()` - Updates only changed pixels (when FAST_REFRESH enabled)
  - `forceCompleteRefresh()` - Clears screen and forces full redraw

### State Management Pattern
- **Global Variables**: All state stored in file-scope globals (time, sensor data, display settings)
- **Flag-Based Dirty Tracking**:
  - `forceFullRedraw` - triggers complete screen clear and redraw
  - `settingsChanged` - enables detailed debug output on next update
  - `lastSecond` - tracks second changes to trigger time updates
- **Non-Blocking Timing**: `millis()` comparisons for sensor updates, NTP sync, status prints
- **No Interrupts**: Polling-based architecture (BOOT button, WiFi, sensor)
- **Display Modes**: Three modes cycle automatically
  - Mode 0: Time + Temperature (displayTimeAndTemp) - shows time with colon, AM/PM if 12h, temp/humidity below
  - Mode 1: Large Time (displayTimeLarge) - 16-pixel tall time with small seconds on right
  - Mode 2: Time + Date (displayTimeAndDate) - time with seconds, date below in selected format
- **Mode Switching**: Automatic cycling every `modeSwitchInterval` seconds (default 5s, range 1-60s)

### Web Server Architecture
- **RESTful Endpoints**:
  - `/` - Main HTML dashboard (server-side rendered, full HTML string)
  - `/api/time` - JSON time data (polled every 1000ms by client)
  - `/api/display` - JSON canvas data for display mirror (polled every 500ms)
  - `/temperature`, `/timeformat`, `/leadingzero`, `/dateformat`, `/timezone`, `/style`, `/rotation`, `/modeinterval` - POST endpoints with 302 redirect to `/`
  - `/reset` - WiFi reset endpoint (clears credentials and restarts)
- **No JavaScript Framework**: Vanilla JS with `setInterval()` for polling
- **Canvas Mirror**: JavaScript redraws TFT display state from `/api/display` every 500ms
- **CORS**: Not implemented (single-origin design)
- **No Persistence**: All settings are volatile (reset on power cycle)

### WiFi Configuration Pattern
- **WiFiManager Library**: Captive portal at 192.168.4.1 ("CYD_Clock_Setup" AP)
- **Timeout**: 180 seconds (3 minutes) before restart if not configured
- **Three Reset Methods**:
  1. BOOT button: Hold 3s during power-up (LED turns yellow→red, displays "WIFI RST" on TFT)
  2. Web UI: `/reset` endpoint (displays confirmation, clears settings, restarts ESP)
  3. Fresh flash: Auto-starts config portal if no saved credentials
- **Callback Hook**: `configModeCallback()` sets RGB LED to purple during AP mode
- **Auto-Reconnect**: WiFi checked in loop(), attempts `WiFi.reconnect()` with 5s timeout, then `ESP.restart()` if failed
- **LED Indicators**:
  - Yellow (steady): BOOT button detected, waiting for 3s hold
  - Red (steady): WiFi reset confirmed
  - Blue (steady): Connecting to WiFi
  - Purple (steady): Config portal active (AP mode)
  - Green (flash): WiFi connected successfully
  - Red (flash): Connection failure

### Time Management
- **NTP Sync**: ESP32 `configTzTime()` with three pool.ntp.org servers
- **Timezone Handling**: POSIX TZ strings from timezones.h (e.g., "AEST-10AEDT,M10.1.0,M4.1.0/3")
- **Timezone Count**: 87 global cities organized by region (Australia, North America, Europe, Asia, South America, Africa, Middle East, Southeast Asia)
- **DST Automatic**: ESP32 SDK handles DST transitions based on POSIX rules
- **struct tm**: Uses ESP32 `localtime()` for calendar calculations
- **Hourly Resync**: NTP sync every 3600s (1 hour) to prevent drift
- **Green LED Flash**: Indicates successful NTP sync
- **Red LED Flash**: Indicates NTP sync failure

### Sensor Abstraction
- **Compile-Time Selection**: `#define USE_BME280` / `USE_SHT3X` / `USE_HTU21D` at src/cyd_tft_clock.cpp:82-85
- **Conditional Compilation**: Only one sensor library linked based on define
- **Runtime Detection**: `testSensor()` attempts I2C communication at known addresses
  - BME280: Tries 0x76 and 0x77
  - SHT3X: Tries 0x44 and 0x45
  - HTU21D: Uses fixed 0x40
- **Graceful Degradation**: Displays "NO SENSOR" if detection fails (clock continues working)
- **Update Interval**: 60s (sensors typically have ~1s measurement time)
- **Sensor Type Display**: Web interface shows which sensor is configured and detected
- **Units Toggle**: Temperature can be switched between °C and °F via web interface

### Memory Considerations
- **PROGMEM Fonts**: Bitmap fonts stored in flash, not RAM
- **String Literals**: HTML served from compiled strings (no SPIFFS/LittleFS overhead)
- **Fixed Buffers**: 64-byte display buffer, no dynamic allocation in main loop
- **Heap Monitoring**: Free heap printed every 60s to detect memory leaks

## Known Issues

### LED Size vs. Content Display
- **Issue**: Large LED sizes (10-12px) may truncate seconds in 24-hour mode with hours ≥ 10
- **Workaround**: Use LED size 4-9px for full seconds display
- **Affected Modes**: Mode 1 (Large Time), Mode 2 (Time+Date)
- **Root Cause**: 32-pixel virtual width limits content when physical LED size is large
- **Note**: Documented in code at src/cyd_tft_clock.cpp:138, 749, 797

### Date Format and LED Size
- **Issue**: Long date formats (YYYY-MM-DD, DD.MM.YYYY, MM.DD.YYYY) require smaller LED size to fit
- **Current Behavior**: Code automatically adjusts LED size (9px for short formats, 8px for long formats) as of v3.6
- **Note**: Date format changes require mode switch interval to cycle back to Mode 2 to see effect
- **User Control**: Users can manually override LED size via web interface if desired

### OTA Upload Safety
- **Issue**: Previously could accidentally OTA upload when USB cable connected
- **Solution**: `upload_port` now commented out in platformio.ini (v3.6)
- **Usage**: Must explicitly specify `--upload-port IP` for OTA or `--upload-port /dev/cu.usbserial-xxx --upload-protocol esptool` for USB
- **Note**: Prevents device bricking during development

### Touchscreen Not Used
- **Issue**: XPT2046 touchscreen hardware present but not utilized
- **Reason**: No user interaction currently required (all config via web UI)
- **Future**: Could add touch-based mode switching or settings

### Single Sensor Limitation
- **Issue**: Only one sensor type can be compiled in at a time
- **Reason**: Simplifies code, reduces flash size, avoids I2C address conflicts
- **Workaround**: Edit src/cyd_tft_clock.cpp:82-85 and recompile for different sensor

## TODO

### From Code Header (src/cyd_tft_clock.cpp:52-57)
- [ ] Enable remote firmware upload via web interface (not just OTA)
  - Currently requires PlatformIO CLI or Arduino IDE for OTA
  - Would need web form to accept .bin file and flash via Update library
- [ ] Add support for additional display modes
  - Potential: Stopwatch, timer, world clock, weather display
  - Could cycle through more than 3 modes
- [ ] Implement more advanced timezone handling
  - Potential: Dual timezone display, automatic location-based timezone
  - Could integrate with geolocation API
- [ ] Add external weather API support
  - Fetch weather data from OpenWeatherMap, Weather.gov, etc.
  - Display forecast, conditions, icons
  - Requires internet connectivity (already have WiFi)
- [ ] Make code portable to other ESP32 and TFT boards
  - Abstract pin definitions
  - Support different TFT drivers (ST7789, ST7735, etc.)
  - Create board selection mechanism

### Potential Enhancements (Not Documented)
- [ ] Touch-based mode switching (utilize XPT2046 touchscreen)
- [ ] MQTT integration for Home Assistant
- [ ] Brightness control (PWM on backlight pin)
- [ ] Alarm/timer functionality with buzzer
- [ ] Multi-sensor support (runtime detection of multiple I2C sensors)
- [ ] SPIFFS/LittleFS for persistent web assets (reduce flash size)
- [ ] WebSocket for live updates (reduce polling overhead)
- [ ] Configurable color themes beyond 8 presets (RGB picker)
- [ ] Animated transitions between display modes
- [ ] Power monitoring (track uptime, WiFi disconnects, reboots)

### Code Quality Improvements
- [ ] Refactor 1800-line main file into multiple modules
  - display.cpp, webserver.cpp, sensor.cpp, time.cpp
- [ ] Move HTML generation to separate file or SPIFFS
- [ ] Add unit tests for font rendering, time formatting
- [ ] Implement settings persistence via Preferences library (currently all settings reset on power cycle)
- [ ] Add input validation for web form parameters

### Documentation
- [x] Create CLAUDE.md for AI-assisted development
- [ ] Add Doxygen comments for API documentation
- [ ] Create troubleshooting flowchart
- [ ] Document I2C address scanning procedure
- [ ] Add video demo of WiFi setup process

---

## Startup Sequence

The device displays status messages on the TFT during boot (see src/cyd_tft_clock.cpp:1810-2018):

1. **"INIT"** - TFT initialized, sensor detection starting
2. **"RESET" + "WIFI"** - (Only if BOOT button held 3s) WiFi credentials being cleared
3. **"WIFI"** - Connecting to saved WiFi network
4. **"SETUP AP"** - (If no WiFi saved) Config portal active at 192.168.4.1
5. **"WIFI OK"** - WiFi connected successfully
6. **IP Address** - Shows "IP:xxx.xxx." (top) and "xxx.xxx" (bottom) for 2.5 seconds
7. **"NTP"** - Syncing time with NTP servers
8. **"TIME OK"** - Time synchronized successfully
9. **"READY"** - Setup complete, clock display starting
10. **OTA Messages** - During OTA update: "OTA", "OTA 10%", "OTA 20%", etc., "OTA OK" or "OTA ERR"

**RGB LED Indicators During Startup**:
- Blue flash (500ms): Device startup
- Yellow steady: BOOT button detected, waiting for 3s hold
- Red steady: WiFi reset confirmed after 3s hold
- Blue steady: Connecting to WiFi
- Purple steady: Config portal active (AP mode)
- Green flash: Success events (WiFi connected, sensor found, NTP synced)
- Yellow flash: Sensor not found warning
- Red flash: Error/failure (WiFi, NTP, OTA)

## Development Workflow

### Initial Setup
1. Clone repository
2. Open in VSCode with PlatformIO
3. Edit `src/cyd_tft_clock.cpp:82-85` to select sensor type
4. Build: `pio run` or Cmd+Shift+B (macOS)
5. USB Upload: `pio run -t upload --upload-port /dev/cu.usbserial-xxx --upload-protocol esptool`
6. Monitor: `pio device monitor` (115200 baud)

### OTA Updates (After Initial Flash)
1. Build: `pio run` or Cmd+Shift+B
2. OTA Upload: `pio run -t upload --upload-port 192.168.1.xxx` or Cmd+U (macOS, edit tasks.json IP)
3. Monitor: Serial monitor at 115200 baud or web interface

### WiFi Reset Procedure (Method 1: BOOT Button)
1. **Power off** the device
2. **Hold down** the BOOT button (GPIO 0)
3. **Power on** while holding BOOT button
4. **Keep holding** - LED will turn yellow (indicates button detected)
5. **Continue holding for 3 seconds** - LED will turn red (confirms WiFi reset)
6. **Release button** - TFT displays "RESET" then "WIFI"
7. Device enters config portal mode (purple LED)
8. Connect to **"CYD_Clock_Setup"** WiFi network from phone/computer
9. Browser should auto-open to **192.168.4.1** (or navigate manually)
10. Select your WiFi network and enter password
11. Click **Save** - device will restart and connect

### WiFi Reset Procedure (Method 2: Web Interface)
1. Navigate to `http://<device-ip>/reset`
2. Device displays confirmation message
3. WiFi credentials are cleared
4. Device restarts in config portal mode
5. Follow steps 8-11 above

### Debug Serial Output
- Connection: 115200 baud, 8N1
- Output: Startup banner, WiFi diagnostics, time/temp every second, status every 60s
- Disable: Set `DEBUG_ENABLED 0` in src/cyd_tft_clock.cpp:175

### Adding a Timezone
1. Find POSIX TZ string for your location (search "POSIX timezone [city]")
2. Edit `include/timezones.h`
3. Add entry to appropriate region in `timezones[]` array:
   ```cpp
   {"City, Country", "TZ_STRING"},
   ```
4. Count is automatic via `sizeof(timezones) / sizeof(timezones[0])`
5. Rebuild and upload via USB or OTA
6. New timezone will appear in web interface dropdown

**Example POSIX TZ String**: `"AEST-10AEDT,M10.1.0,M4.1.0/3"`
- `AEST` = Standard time abbreviation
- `-10` = UTC offset (negative for east of UTC)
- `AEDT` = Daylight saving time abbreviation
- `M10.1.0` = DST starts month 10, week 1, day 0 (Sunday)
- `M4.1.0/3` = DST ends month 4, week 1, Sunday at 3am

### Changing OTA Password
1. Edit `platformio.ini:26` - change `--auth=CYD_OTA_2024`
2. Edit `src/cyd_tft_clock.cpp:1965` - change `ArduinoOTA.setPassword("CYD_OTA_2024")`
3. **CRITICAL**: Password must match in both locations
4. Upload via USB first (OTA will fail with old password after code change)

---

**Last Updated**: 2026-01-16
**AI Assistant**: Claude Code (Sonnet 4.5)
**Developer**: Anthony Clarke
