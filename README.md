# ESP8266 TFT LED Matrix Clock

A refactored version of the ESP8266 LED Matrix Clock that uses SPI TFT displays (ILI9341/ST7789) to **simulate** the appearance of MAX7219 LED matrix modules with **realistic circular LED rendering**.
All functionality from the original LED matrix version is preserved and enhanced. (https://github.com/anthonyjclarke/ESP_LEDMatrix_32x16_NTP_Clock)

## Features

- 🎨 **Realistic LED Matrix Simulation** - Two display styles:
  - **Default**: Fast solid blocks
  - **Realistic**: Circular LEDs with customizable surrounds matching real MAX7219 hardware
- ⏰ **Three Display Modes**: Time+Temperature, Large Time, Time+Date (all with seconds)
- 🕐 **12/24 Hour Format**: Toggle between formats via web interface
- 🌡️ **BME280 Sensor Integration**: Temperature, humidity, and pressure readings
- 📡 **WiFi Manager**: Easy setup without hardcoded credentials
- 🌍 **NTP Time Sync**: Automatic time synchronization with 88 timezone options
- 🌐 **Modern Responsive Web Interface**:
  - Live-updating clock (refreshes every second)
  - Dynamic weather icons based on temperature
  - Mobile/tablet/desktop responsive design
  - Dark theme with glowing effects
- 🎨 **Customizable Colors**: 8 LED colors and 8 surround colors (including "Match LED Color")
- ⚡ **Instant Refresh**: Style and color changes apply immediately

## Display Simulation

### Realistic LED Style
The TFT display renders authentic-looking circular LEDs to replicate real MAX7219 hardware:
- **Circular LED pixels** with customizable colors (8 color options)
- **Visible off LEDs** as dark circles (like real hardware)
- **Prominent surround/bezel rings** (8 options including matching LED color)
- **Authentic 4-pixel gap** between matrix rows
- **Full-brightness surrounds** for clear color distinction and visibility
- **Proper color dimming** that preserves hue in RGB565 format
- **Optimized rendering** with wider bezel rings for better appearance

### Display Styles
**Default Style (Fast):**
- Solid square LED blocks
- Fastest rendering (~20ms updates)
- Clean, modern appearance

**Realistic Style (Authentic):**
- Circular LEDs with surround bezels
- Matches real MAX7219 appearance
- Visible grid structure even when LEDs are off
- Slower rendering (~300-400ms) but 6-8× faster than original

## Hardware Requirements

### Required Components

1. **ESP8266 Development Board**
   - D1 Mini, NodeMCU, or similar
   - Minimum 80KB RAM

2. **TFT Display** (Choose ONE):
   - 1.8" ILI9341 (240x320) SPI TFT Display
   - 2.4" ILI9341 (240x320) SPI TFT Display
   - 2.8" ILI9341 (240x320) SPI TFT Display
   - 2.4" ST7789 (240x320) SPI TFT Display

3. **BME280 Sensor** (Optional but recommended)
   - I2C Temperature/Humidity/Pressure sensor
   - Default address: 0x76 (can be 0x77)

### Components NOT Required (TFT Version)
- ❌ **PIR Motion Sensor** - Display always on in TFT version
- ❌ **LDR Light Sensor** - Fixed brightness in TFT version

### Optional Components

- Breadboard or PCB
- Jumper wires
- 5V power supply (1A minimum)

## Wiring Diagram

### TFT Display (ILI9341/ST7789) - SPI Connection

```
ESP8266 D1 Mini  →  TFT Display
─────────────────────────────────
D8 (GPIO15)      →  CS  (Chip Select)
D3 (GPIO0)       →  DC  (Data/Command)
D4 (GPIO2)       →  RST (Reset)
D7 (GPIO13)      →  MOSI/SDA (Data)
D5 (GPIO14)      →  SCK/SCL (Clock)
3.3V             →  VCC
3.3V             →  LED (Backlight) *
GND              →  GND

* Note: Some TFT modules have built-in backlight control.
  If your module has a separate LED pin, connect it to 3.3V
  or to a PWM pin for brightness control.
```

### BME280 Sensor - I2C Connection

```
ESP8266 D1 Mini  →  BME280
────────────────────────────
D4 (GPIO4)       →  SDA
D3 (GPIO5)       →  SCL
3.3V             →  VCC
GND              →  GND
```

### Complete Wiring Table

| Component      | Pin Name  | ESP8266 Pin | GPIO | Notes                    |
|----------------|-----------|-------------|------|--------------------------|
| **TFT Display**|           |             |      |                          |
| CS             | TFT_CS    | D8          | 15   | Chip Select              |
| DC             | TFT_DC    | D3          | 0    | Data/Command             |
| RST            | TFT_RST   | D4          | 2    | Reset                    |
| MOSI           | MOSI      | D7          | 13   | Hardware SPI             |
| SCK            | SCK       | D5          | 14   | Hardware SPI             |
| **BME280**     |           |             |      |                          |
| SDA            | SDA       | D4          | 4    | I2C Data                 |
| SCL            | SCL       | D3          | 5    | I2C Clock                |

**Note:** PIR (D0) and LDR (A0) pins are NOT used in TFT version.

## Display Configuration

### Supported Display Types

The code supports both ILI9341 and ST7789 displays. Choose your display type in `main_tft.cpp`:

```cpp
// Uncomment ONE of the following:
#define USE_ILI9341    // For ILI9341 displays (most common)
// #define USE_ST7789  // For ST7789 displays
```

### Display Customization

#### LED Colors (8 Options)
- Red (default)
- Green
- Blue
- Yellow
- Cyan
- Magenta
- White
- Orange

#### Surround/Bezel Colors (8 Options)
- White
- Light Gray
- Dark Gray (default - authentic MAX7219 look)
- Red
- Green
- Blue
- Yellow
- **Match LED Color** (surrounds match LED color)

#### Display Styles
- **Default (0)**: Solid square blocks (fastest)
- **Realistic (1)**: Circular LEDs with surrounds (authentic)

You can adjust these settings via the web interface or modify constants in `main_tft.cpp`:

```cpp
#define LED_SIZE          10     // Size of each LED (10 pixels for 32-wide display)
#define DEFAULT_DISPLAY_STYLE 1  // 0=Default, 1=Realistic
```

### Color Options (RGB565 format)

```cpp
// Predefined colors:
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_WHITE       0xFFFF
#define COLOR_ORANGE      0xFD20
#define COLOR_DARK_GRAY   0x7BEF
#define COLOR_LIGHT_GRAY  0xC618
```

## Software Setup

### 1. Install PlatformIO

Install PlatformIO IDE (VS Code extension) or PlatformIO Core:
- **VS Code**: Install "PlatformIO IDE" extension
- **Command Line**: `pip install platformio`

### 2. Clone/Download Project

```bash
git clone <your-repo-url>
cd tft-led-clock
```

### 3. Configure Display Type

Edit `main_tft.cpp` and select your display:

```cpp
#define USE_ILI9341    // Most common 2.4" and 2.8" displays
// #define USE_ST7789  // Some 1.8" and 2.4" displays
```

### 4. Verify Pin Connections

Check that the pin definitions in `main_tft.cpp` match your wiring:

```cpp
#define TFT_CS    D8
#define TFT_DC    D3
#define TFT_RST   D4
#define SDA_PIN   D4
#define SCL_PIN   D3
```

### 5. Build and Upload

Using PlatformIO:

```bash
# Build the project
pio run

# Upload to ESP8266
pio run --target upload

# Monitor serial output
pio device monitor
```

Or use the PlatformIO IDE buttons in VS Code.

### 6. Initial WiFi Setup

1. Power on the ESP8266
2. Look for WiFi network: **TFT_Clock_Setup**
3. Connect to it with your phone/computer
4. Browser should auto-open to configuration portal
5. Select your WiFi network and enter password
6. Click "Save"

The display will show status messages during setup:
- `INIT` - Initializing display
- `WIFI OK` - WiFi connected
- `TIME OK` - NTP sync successful
- `READY` - System ready

## Web Interface

After WiFi setup, access the web interface:

```
http://<esp8266-ip-address>
```

Find the IP address from:
- Serial monitor output
- Your router's DHCP client list
- mDNS: `http://tft-clock.local` (if mDNS is working)

### Web Interface Features

- **Live Digital Clock Display**:
  - Large, glowing clock with auto-update every second (no page refresh needed)
  - Color-coded date display
  - Modern dark theme with gradient effects

- **Dynamic Environment Dashboard**:
  - Temperature with weather emoji icons (changes based on actual temp)
  - Humidity with condition-based icons
  - Pressure monitoring
  - Color-coded values with glowing effects
  - Responsive grid layout

- **Display Style Controls**:
  - Toggle between Default/Realistic styles
  - Select LED color (8 options)
  - Select surround color (8 options including "Match LED Color")
  - Instant visual feedback on changes

- **Time Format**: Toggle between 12/24 hour format
- **Settings**: Toggle temperature unit (°C/°F)
- **Timezone & Time Format**: 88 timezones + 12/24 hour selection
- **System Info**: IP address, uptime, WiFi reset

- **Responsive Design**:
  - Adapts to mobile phones, tablets, and desktops
  - Touch-friendly buttons and controls
  - Fluid typography that scales with screen size

## Display Modes

The clock automatically cycles through three display modes every 5 seconds:

### Mode 0: Time + Temperature/Humidity
```
10:24 58    ← Time with seconds
T26C H88%   ← Temperature & Humidity
```
- Shows time with seconds
- Temperature, humidity on bottom row
- Seconds hidden in 24-hour mode when hours ≥ 10 (space constraint)

### Mode 1: Large Time
```
10:24 58    ← Large time with seconds
(empty)     ← Spans both rows
```
- Centered large time display
- Maximum visibility
- Seconds shown in small font

### Mode 2: Time + Date
```
9:24 58     ← Time with seconds (no leading zero)
18/12/24    ← Date (DD/MM/YY)
```
- Time with seconds on top row (no leading zero for single-digit hours)
- Date on bottom row
- Format: DD/MM/YY
- Consistent hour formatting with Mode 0

## Time Format

### 12-Hour Mode (Default)
- Hours: 1-12
- Seconds always shown in all modes
- AM/PM implied by context

### 24-Hour Mode
- Hours: 0-23
- Seconds shown in most cases
- **Mode 0 (Time+Temp)**: Seconds hidden when hours ≥ 10 (not enough space)
- **Mode 1 (Large Time)**: Seconds always shown
- **Mode 2 (Time+Date)**: Seconds always shown

**Note**: When 24-hour mode is active, Mode 0 displays a notice in the web interface about seconds being hidden for times 10:00-23:59.

## Configuration Options

### Display Style

**Default Style:**
- Fast rendering
- Solid square LEDs
- Modern, clean appearance

**Realistic Style:**
- Authentic MAX7219 appearance
- Circular LEDs with surrounds
- Visible off LEDs (dark circles)
- Gray plastic bezels (customizable)
- 4-pixel gap between matrix rows

### LED & Surround Colors

Choose from 8 LED colors and 8 surround colors via web interface.

**Popular Combinations:**
- **Red + Dark Gray**: Classic MAX7219 look
- **Red + Match LED Color**: Monochrome red appearance
- **Green + Dark Gray**: High visibility
- **Cyan + Match LED Color**: Modern cyan-on-cyan

Changes apply instantly with automatic full screen refresh.

### Timezone Configuration

88 supported timezones including:
- All Australian zones (Sydney, Melbourne, Brisbane, Adelaide, Perth, Darwin, Hobart)
- US zones (EST, CST, MST, PST, Alaska, Hawaii)
- European zones (UK, CET, EET)
- Asian zones (Tokyo, Hong Kong, Singapore, Bangkok, Mumbai, Dubai)
- And many more...

## Troubleshooting

### Display Issues

**Display is blank:**
- Check TFT connections (CS, DC, RST, MOSI, SCK)
- Verify correct display type is selected (#define)
- Check TFT power supply (3.3V)
- Check rotation setting: `tft.setRotation(3)` (landscape)

**Display shows garbage:**
- Wrong display type selected
- SPI pins connected incorrectly
- Try different rotation values (0-3)

**LEDs look wrong:**
- Try switching display styles via web interface
- Adjust LED colors via web interface
- Check realistic display settings in code

**Display too dim/bright:**
- TFT backlight is fixed at level 8
- Adjust in code if needed: `setTFTBrightness(level)`

**Colors don't change/refresh slowly:**
- Should be instant with latest code
- Check serial monitor for "Forced full screen clear" message
- Verify `forceFullRedraw` flag is working

**Colon blinks with display shift:**
- Should be fixed in latest code
- Uses 2-pixel space reservation when colon hidden
- Verify consistent spacing in displayTimeAndTemp/displayTimeAndDate

### Sensor Issues

**BME280 not detected:**
- Check I2C connections (SDA=D4, SCL=D3)
- Verify I2C address (0x76 or 0x77)
- Try other address in code: `bme280.begin(0x77)`
- Check I2C pullup resistors (may be required)

### WiFi Issues

**Won't connect to WiFi:**
- Use Reset WiFi button in web interface
- Connect to "TFT_Clock_Setup" AP and reconfigure
- Check WiFi credentials are correct
- Ensure 2.4GHz network (ESP8266 doesn't support 5GHz)

**Web interface not accessible:**
- Check ESP8266 IP address in serial monitor
- Verify ESP8266 and computer on same network
- Try accessing directly by IP: `http://192.168.x.x`

### Time Issues

**Time not syncing:**
- Check WiFi connection
- Verify firewall allows NTP (port 123 UDP)
- Try different NTP server in code
- Check timezone selection

**Wrong timezone:**
- Select correct timezone in web interface
- Verify POSIX TZ string is correct
- Check DST settings for your region

**Seconds not showing:**
- **Mode 0 (Time+Temp)**: Hidden in 24-hour mode when hours ≥ 10
- **Mode 1 (Large Time)**: Should always show
- **Mode 2 (Time+Date)**: Should always show
- Try 12-hour format for consistent seconds display

## Serial Debug Output

Connect to serial monitor (115200 baud) to see:

```
╔═══════════════════════════════════════╗
║   ESP8266 TFT Matrix Clock v2.0       ║
║   TFT Display Edition - Enhanced      ║
╚═══════════════════════════════════════╝

Initializing TFT Display...
TFT Display initialized: 320x240
LED Matrix area: 320x164 (with 4px row gap)
Display Mode: Realistic (circular LEDs)
BME280 OK: 22.5°C, 45.3%
Connected! IP: 192.168.1.100
Syncing time with NTP...
Time synced: 14:23:45 17/12/2025 (TZ: Sydney, Australia)
Web server started

Time: 14:23 | Date: 18/12/2024 | Temp: 23°C | Hum: 45%
```

## Advanced Configuration

### Custom LED Appearance

Adjust LED rendering in `main_tft.cpp`:

```cpp
// LED size (10 pixels = 32 LEDs across 320px screen)
#define LED_SIZE 10

// Default style (0=blocks, 1=realistic)
#define DEFAULT_DISPLAY_STYLE 1

// Row gap (4 pixels matches real MAX7219)
int matrixGap = (y >= 8) ? 4 : 0;

// Circle radii (in drawLEDPixel function) - optimized for visibility
if (distSq <= 18) {      // Inner core (bright)
if (distSq <= 38) {      // LED body (bright)
if (distSq <= 62) {      // Surround ring (full brightness, no dimming)
```

### Adjust Color Dimming

In `dimRGB565()` function:
```cpp
// Dimming factors:
dimRGB565(color, 0) = 100% brightness
dimRGB565(color, 1) = 50% brightness  (for secondary elements)
dimRGB565(color, 7) = 12.5% brightness (used for off LEDs)

// Note: Surrounds now use full brightness for better visibility
```

### Display Rotation

Adjust display orientation:

```cpp
// In initTFT() function:
tft.setRotation(0);  // Portrait
tft.setRotation(1);  // Landscape
tft.setRotation(2);  // Portrait inverted
tft.setRotation(3);  // Landscape inverted (default)
```

## API Endpoints

The web server provides these endpoints:

### GET /
Main web interface with all controls

### GET /style
Toggle display style and colors:
- `mode=toggle` - Switch between Default/Realistic
- `ledcolor=0-7` - Set LED color (0=Red, 1=Green, etc.)
- `surroundcolor=0-7` - Set surround color (7=Match LED Color)

### GET /timeformat
Toggle time format:
- `mode=toggle` - Switch between 12/24 hour format

### GET /temperature
Toggle temperature unit:
- `mode=toggle` - Switch between °C/°F

### GET /timezone
Change timezone:
- `tz=0-87` - Set timezone index

### GET /display
Toggle display on/off:
- `mode=toggle` - Manual display toggle (5-minute override)

### GET /reset
Reset WiFi settings and restart

## Performance Notes

- **Refresh Rate**: 
  - Default style: ~50 FPS (20ms updates)
  - Realistic style: ~3 FPS (300-400ms updates)
- **Fast Refresh**: Only changed pixels are redrawn
- **Color Changes**: Instant refresh (forceFullRedraw mechanism)
- **Power Consumption**: 
  - Active: ~250mA @ 5V
  - Display off: ~80mA @ 5V
- **WiFi**: 2.4GHz only (ESP8266 limitation)
- **Memory**: ~40KB RAM used, ~40KB free

## Changelog

### Version 2.1 (18 December 2025)

#### Major Web Interface Overhaul
- 🎨 **Complete UI Redesign**
  - Modern dark theme with gradient cards and glowing effects
  - Large live-updating digital clock (auto-refreshes every second)
  - No page reload needed for time updates
  - Professional gradient backgrounds and shadows

- 📱 **Fully Responsive Design**
  - Mobile-first approach with touch-friendly controls
  - Fluid typography using CSS clamp() for all screen sizes
  - Adaptive grid layouts for phones, tablets, and desktops
  - Breakpoints: Mobile (<768px), Tablet (769-1024px), Desktop (>1024px)

- 🌡️ **Dynamic Environment Display**
  - Temperature-based weather icons (7 levels):
    * 🔥 Hot (≥30°C), ☀️ Warm (25-29°C), 🌤️ Pleasant (20-24°C)
    * ⛅ Mild (15-19°C), ☁️ Cool (10-14°C), 🌧️ Cold (5-9°C), ❄️ Freezing (<5°C)
  - Dynamic humidity icons: 💦 (high), 💧 (normal), 🏜️ (low)
  - Color-coded sensor values with glowing effects
  - Responsive card grid with hover animations

- 🎯 **Enhanced LED Rendering**
  - Fixed surround color visibility (now properly visible and configurable)
  - Wider, brighter surround ring for better color distinction
  - Removed excessive dimming for clearer color differentiation
  - Full 10x10 pixel LED area utilization

#### Display Mode Improvements
- ⏰ **Consistent Hour Formatting**
  - Mode 2 (Time+Date) now removes leading zero from single-digit hours
  - Matches Mode 0 behavior for uniform appearance
  - Works correctly in both 12-hour and 24-hour formats

#### Performance & UX
- ⚡ Live time updates via JavaScript fetch API
- ⚡ Instant visual feedback on all setting changes
- 🎨 Smooth hover effects and transitions
- 📐 Centered layouts with max-width containers

### Version 2.0 (early December 2025)

#### Major Features Added
- ✅ **Realistic LED Display Style**
  - Circular LEDs with customizable surround colors
  - Authentic MAX7219 appearance
  - Visible off LEDs (dark circles)
  - 4-pixel gap between matrix rows
  - Proper RGB565 color dimming that preserves hue

- ✅ **12/24 Hour Format Toggle**
  - Web interface toggle
  - Smart seconds handling (hidden in 24h mode when space limited)
  - All display modes respect time format preference

- ✅ **Enhanced Color System**
  - 8 LED colors (Red, Green, Blue, Yellow, Cyan, Magenta, White, Orange)
  - 8 surround colors (including "Match LED Color" option)
  - Instant color changes with forced screen refresh

- ✅ **Seconds Display**
  - All three modes now show seconds
  - Small font (digits3x5) for space efficiency
  - Smart hiding when space constrained (24h Mode 0 only)

#### Performance Improvements
- ⚡ 6-8× faster rendering vs original (300-400ms vs 2-3 seconds)
- ⚡ Instant style/color changes (forceFullRedraw flag)
- ⚡ Optimized off LED rendering (fillRect vs 100 drawPixel calls)
- ⚡ Smart FAST_REFRESH cache clearing

#### Bug Fixes
- 🐛 Fixed colon blinking causing display shift
- 🐛 Fixed "Match LED Color" not syncing when LED color changes
- 🐛 Fixed seconds being cut off in large time mode
- 🐛 Fixed bit-shift color dimming (now preserves hue properly)
- 🐛 Fixed slow refresh on style/color changes

#### Code Cleanup
- 🧹 Removed all PIR motion sensor code (not used in TFT version)
- 🧹 Removed all LDR light sensor code (not used in TFT version)
- 🧹 Removed computeAmbientBrightnessFromLdr function
- 🧹 Simplified handleDisplayControl (was handleBrightnessAndMotion)
- 🧹 Cleaned up serial output (removed motion/light level)
- 🧹 Removed unnecessary display status output

#### Web Interface Updates
- 🌐 Added Display Style card with color selectors
- 🌐 Added 12/24 hour format toggle in Timezone section
- 🌐 Simplified Settings card (removed display toggle)
- 🌐 Added warning for 24-hour mode seconds limitation
- 🌐 Instant refresh on all style/color changes

### Version 1.0 (November 2025)
- Initial TFT refactor from LED matrix version
- Basic LED simulation with solid blocks
- Web interface for configuration
- BME280 sensor support
- NTP time sync with 88 timezones
- Three display modes

## Future Enhancements

- [ ] Weather API integration
- [ ] OTA firmware updates  
- [ ] Additional display animations
- [ ] Custom font support
- [ ] MQTT integration
- [ ] Alarm functionality
- [ ] Multiple timezone clocks
- [ ] Adjustable LED glow/bloom effects

## Credits

- Original LED Matrix version by Anthony Clarke
- TFT refactor and enhancements by Anthony Clarke
- Realistic LED rendering inspired by real MAX7219 hardware
- Based on MAX7219 concepts by Pawel A. Hernik
- Font data & code / idea from original LED matrix project by https://www.youtube.com/@cbm80amiga

## License

MIT License - Feel free to modify and share!

## Support

For issues, questions, or contributions:
- Open an issue on GitHub
- Check the troubleshooting section above
- Review serial debug output for diagnostics
- Consult the changelog for recent changes

---

**Enjoy your realistic TFT LED Matrix Clock!** 🎨⏰✨