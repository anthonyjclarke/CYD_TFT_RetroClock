# Changelog

All notable changes to the ESP32 CYD TFT Matrix Clock project will be documented in this file.

## [3.7.0] 09-10-2026

### Added
- **Browser installer** at https://anthonyjclarke.github.io/CYD_TFT_RetroClock/
  (ESP Web Tools): install or update from Chrome/Edge, no PlatformIO needed.
- **Improv-Serial, always on**: the installer's **Configure WiFi** sets WiFi
  over USB, and a board running this firmware is offered **Update**, which
  keeps WiFi. Vendored library in `lib/ImprovWiFi/`; the WiFiManager portal
  now runs non-blocking so Improv is served alongside it.
- **Release workflow** (`.github/workflows/firmware.yml`, shared
  `cyd-web-installer`): every push builds; a `v*` tag publishes
  `*-firmware.bin`, `*-merged.bin`, `SHA256SUMS.txt` and the installer page.
- `include/config.h` with `FIRMWARE_VERSION` and `PROJECT_NAME`; the serial
  banner shows the version. Boot log line `Running from app0|app1`.
- `tools/merge_bin.py` post-build script (flash parts + merged image).

### Fixed
- Improv library synced with cyd-web-installer 1.0.1: each packet starts on a
  new line, so serial noise on port open no longer makes Connect offer
  Install instead of Update.

### Changed
- **Partition table**: `default.csv` (1.25 MB app slots) replaced by the
  standard dual-OTA `partitions_custom.csv` (1.79 MB slots). NVS keeps its
  offset, so WiFi survives. A 3.6 board has no Improv, so the installer
  offers Install with an erase prompt: say no to keep WiFi.
- **Platform pinned** to `espressif32@6.12.0` (unpinned now resolves to
  pioarduino 3.x, which does not build this project).
- WiFiManager AP name moved to `AP_NAME` in `config.h` (still
  `CYD_Clock_Setup`).

## [3.6] - 2026-01-08

### Added
- **Date Format Selector**:
  - 5 common date formats selectable via web interface
  - Formats: DD/MM/YY, MM/DD/YY, YY/MM/DD, DD/MM/YYYY, MM/DD/YYYY
  - Real-time switching without restart
  - Displayed in Mode 2 (Time + Date)
  - LED Size automatically adjusts based on format (9px for short, 8px for long)

- **Display Rotation Control**:
  - Flip display 180° via web interface toggle
  - Rotation 1 (normal) vs Rotation 3 (flipped)
  - Useful for different mounting orientations
  - Setting persists across reboots

- **VS Code Development Tools**:
  - Custom tasks for PlatformIO workflow in [.vscode/tasks.json](.vscode/tasks.json)
    - "PlatformIO: Upload (OTA)" - Upload via OTA to specified IP
    - "PlatformIO: Build Only (No Upload)" - Build without uploading
  - Keyboard shortcuts in [.vscode/keybindings.json](.vscode/keybindings.json)
    - `Cmd+U` (macOS) - OTA Upload
    - `Cmd+Shift+B` - Build Only
  - Improved developer experience for PlatformIO users

### Changed
- **Default Display Rotation**: Changed from 1 (normal) to 3 (180° flipped)
  - Users can toggle back to rotation 1 via web interface if needed
  - Provides better default orientation for common mounting scenarios

- **OTA Upload Safety**:
  - `upload_port` now commented out in [platformio.ini](platformio.ini) by default
  - Prevents accidental OTA uploads when USB cable is connected
  - Users must explicitly specify target with `--upload-port` flag
  - Clear documentation on OTA vs USB upload procedures
  - Reduces risk of bricking device during development

### Documentation
- Added comprehensive comments in [platformio.ini](platformio.ini) explaining OTA safety measures
- Added note in CHANGELOG about LED Size adjustment for different date formats
- Documented Upload icon issue workaround in VS Code tasks

## [3.5] - 2026-01-06

### Added
- **Mode Switch Interval Control**:
  - User-configurable display mode duration (1-60 seconds, default: 5)
  - Web interface slider control in Display Customization section
  - Real-time adjustment without restart

- **Footer Panel on Web Interface**:
  - Links to GitHub repository and Bluesky profile
  - Attribution: "Built with ❤️ by Anthony Clarke"
  - Credit to original creator @cbm80amiga with link to original project
  - Responsive design matching site theme

- **Flashing Colon in Mode 0**:
  - Colon now flashes every second in Time+Temp mode (Mode 0)
  - Matches behavior of Mode 1 and Mode 2
  - Uses standard `showDots` variable tied to seconds

### Changed
- **Mode 0 Font Consistency**:
  - All text now uses `font3x7` (7-pixel height) for uniform appearance
  - Hours, minutes, colon, AM/PM, temperature, and humidity all same size
  - Improved visual coherence in Time+Temp display

- **Debug Output Optimization**:
  - Consolidated debug messages to reduce serial clutter
  - Status interval increased from 10s to 60s
  - New `DEBUG_SETTINGS` macro shows detailed output only when settings change
  - Mode display now shows actual content being displayed
  - Settings changes trigger one-time detailed output

- **UI Layout Improvements**:
  - Mode Switch Interval slider moved from "Timezone & Time Format" to "Display Customization"
  - Positioned below LED Spacing for logical grouping
  - Consistent slider styling across all controls

### Fixed
- **Font Character Support**:
  - Fixed `font3x7` colon character bitmap from `0x00` to `0x24` (binary: 00100100)
  - Colon now displays properly with two vertically-aligned dots
  - AM/PM now uses `font3x7` instead of `digits3x5` (which only has 0-9)

### Documentation
- Added comprehensive acknowledgements section in code header
- Referenced original ESP8266 project by @cbm80amiga (Pawel A.)
- Links to original YouTube video and GitHub repository
- Clear explanation of refactoring and enhancements

## [3.0] - 2026-01-05

### Added
- **Multi-Sensor Support**: Added compile-time configuration for three environmental sensors:
  - BME280 (Temperature, Humidity, Pressure)
  - SHT3X (Temperature, Humidity)
  - HTU21D (Temperature, Humidity)
  - Auto-detection of I2C addresses for each sensor type
  - Sensor information displayed in web interface System panel

- **Dynamic LED Configuration**:
  - LED Size adjustable via web interface (4-12 pixels, default: 9px)
  - LED Spacing adjustable via web interface (0-3 pixels, default: 1px)
  - Real-time adjustment without recompilation
  - Helps prevent seconds truncation in 24-hour mode

- **Leading Zero Display Option**:
  - Toggle leading zero for hours < 10 via web interface
  - OFF: "1:23:45" or "1:23 PM"
  - ON: "01:23:45" or "01:23 PM"

- **AM/PM Indicator in Mode 0**:
  - In 12-hour mode, displays "AM" or "PM" instead of seconds
  - Provides clearer time period indication
  - Seconds removed entirely from Mode 0 (Time+Temp display)

- **Enhanced Timezone Selection**:
  - 87 timezones reorganized into HTML optgroups by region
  - Non-selectable region headers for easier navigation
  - Regions: Australia & Oceania, North America, South America, Western Europe, Northern Europe, Central & Eastern Europe, Middle East, South Asia, Southeast Asia, East Asia, Central Asia, Caucasus, Africa
  - Default timezone: Sydney, Australia (index 0)

- **Helpful UI Tips**:
  - Added tip below TFT Display Mirror: "💡 Tip: If seconds are truncated, adjust LED Size or Spacing below"
  - Code comments explaining truncation behavior and solutions

### Changed
- **Optimized Web Interface Spacing**:
  - Reduced CSS padding and margins throughout for more compact layout
  - Body padding: 15px → 10px
  - Card margins: 10px → 8px
  - Card padding: 20px → 16px
  - Smaller font sizes for better information density
  - Improved mobile responsiveness

- **Colon Spacing Adjustments**:
  - Mode 0 (Time+Temp): 1 LED space before colon
  - Mode 1 (Large Time): 1 LED space before AND after colon
  - Mode 2 (Time+Date): 1 LED space before colon

- **Default LED Size**: Reduced from 10px to 9px for better content fit
  - 9px × 32 LEDs = 288px width (leaves margin on 320px display)
  - Prevents seconds truncation in most scenarios

- **Sensor Information Display**:
  - Moved sensor details from standalone card to System panel
  - Compact inline format showing sensor type, capabilities, and I2C addresses

### Fixed
- **Seconds Display in 24-Hour Mode**:
  - Removed `canShowSeconds` restriction that prevented seconds from displaying when hours >= 10
  - Updated boundary checks from `LINE_WIDTH - 3` to `LINE_WIDTH` in all display modes
  - Seconds now always attempt to display (clipped naturally if no room)

- **Timezone Array Structure**:
  - Removed fake separator entries that were selectable options
  - Clean array structure with proper regional organization

### Documentation
- Updated README.md with:
  - Comprehensive sensor configuration section
  - I2C address information for all sensors
  - Hardware reference image location
  - Improved troubleshooting section

## [3.0] - Previous Release

Initial ESP32 CYD version with:
- MAX7219-style LED matrix simulation
- WiFi Manager with BOOT button reset
- NTP time synchronization
- 88 global timezones
- BME280 sensor support
- Web interface with live display mirror
- RGB LED status indicators
- Serial diagnostics

---

**Note**: Version numbers follow semantic versioning. This changelog follows the format from [Keep a Changelog](https://keepachangelog.com/).
