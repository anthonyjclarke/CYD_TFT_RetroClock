# Changelog

All notable changes to the ESP32 CYD TFT Matrix Clock project will be documented in this file.

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
