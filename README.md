# ESP32 CYD TFT Matrix Clock

A retro LED matrix clock simulator running on the ESP32 Cheap Yellow Display (CYD) board.

## 📸 Display Modes

The clock cycles through three distinct display modes, each optimized for different information display:

### Mode 0: Time + Temperature
![Mode 0: Time + Temperature](images/mode0_time_temp.png)

- **Top Row**: Current time in 12/24-hour format with flashing colon
- **Bottom Row**: Temperature and humidity readings from connected sensor
- **Features**: Shows AM/PM indicator in 12-hour mode, no seconds displayed
- **Example**: `12:47 PM` with `T28°C H47%`

### Mode 1: Large Time Display
![Mode 1: Large Time Display](images/mode1_large_time.png)

- **Display**: Large 16-pixel tall time spanning both rows
- **Seconds**: Displayed in small font on the right
- **Features**: Maximum visibility for time, perfect for across-the-room viewing
- **Example**: `12:47:27` with prominent hours and minutes

### Mode 2: Time + Date
![Mode 2: Time + Date](images/mode2_time_date.png)

- **Top Row**: Current time with seconds in small font
- **Bottom Row**: Date in DD/MM/YY format
- **Features**: Complete time and date information at a glance
- **Example**: `12:47:32` with `06/01/26` (January 6, 2026)

**Mode Switching**: The display automatically cycles through all three modes at a configurable interval (default: 5 seconds, adjustable 1-60 seconds via web interface). The flashing colon in all modes provides a visual heartbeat indicator.

## Hardware

### Board: ESP32-2432S028R (Cheap Yellow Display)

![ESP32 CYD Board Reference](images/Reference_CYD.jpeg)

The CYD is an affordable ESP32 development board with integrated:
- 2.8" ILI9341 TFT display (320x240 pixels)
- XPT2046 resistive touchscreen
- RGB LED (active low)
- SD card slot
- Extended GPIO connector

### Pin Configuration

| Function | GPIO | Notes |
|----------|------|-------|
| **TFT Display (HSPI)** | | |
| MOSI | 13 | Data out |
| MISO | 12 | Data in (not used) |
| CLK | 14 | SPI clock |
| CS | 15 | Chip select |
| DC | 2 | Data/Command |
| RST | -1 | Connected to EN |
| Backlight | 21 | HIGH = on |
| **RGB LED** | | Active LOW |
| Red | 4 | |
| Green | 16 | |
| Blue | 17 | |
| **Boot Button** | 0 | Built-in, active LOW |
| **I2C (Extended GPIO)** | | For environmental sensors |
| SDA | 27 | Via CN1 connector |
| SCL | 22 | Via CN1 connector |
| **Touchscreen (VSPI)** | | Not used in this project |
| T_IRQ | 36 | |
| T_DIN | 32 | |
| T_OUT | 39 | |
| T_CLK | 25 | |
| T_CS | 33 | |

## Features

- **Simulated LED Matrix**: Authentic MAX7219-style 4x2 LED matrix appearance
- **WiFi Manager**: Easy WiFi configuration via captive portal
  - BOOT button reset (3-second hold during power-up)
  - Web interface reset option
  - Automatic config portal on first boot
- **Network Diagnostics**: Comprehensive WiFi monitoring and auto-reconnect
- **IP Address Display**: Shows IP on TFT during startup (2.5 seconds)
- **NTP Time Sync**: Automatic time synchronization with DST support
- **87 Timezones**: Comprehensive global timezone support organized by region
- **Environmental Sensors**: Support for BME280 (temp/humidity/pressure), SHT3X, or HTU21D (temp/humidity)
- **Web Interface**: Modern responsive control panel with live display mirror
- **Display Customization**:
  - Display styles: Default blocks or realistic circular LEDs
  - 8 LED colors: Red, Green, Blue, Yellow, Cyan, Magenta, White, Orange
  - Adjustable LED size (4-12 pixels, default: 9px)
  - Adjustable LED spacing (0-3 pixels, default: 1px)
  - Configurable mode switch interval (1-60 seconds, default: 5s)
- **Time Display Options**:
  - 12/24-hour format toggle
  - Leading zero toggle for hours < 10
  - Flashing colon in all modes
- **RGB LED Status**: Visual feedback during startup and operation
- **Serial Diagnostics**: Optimized real-time monitoring at 115200 baud

## Display Modes

The clock cycles through three display modes (interval configurable via web interface, default: 5 seconds):

1. **Mode 0 - Time + Temperature**: Time with flashing colon (and AM/PM in 12-hour mode) on top row, temp/humidity on bottom. Seconds are not displayed in this mode.
2. **Mode 1 - Large Time**: Full 16-pixel tall time display with flashing colon and seconds in small font
3. **Mode 2 - Time + Date**: Time with flashing colon and seconds on top row, date (DD/MM/YY) on bottom

## Startup Sequence

The TFT display shows the following messages during boot:

1. **"INIT"** - Initializing hardware
2. **"RESET"** + **"WIFI"** - (Only if BOOT button held) WiFi reset in progress
3. **"WIFI"** - Connecting to WiFi
4. **"SETUP AP"** - (If no WiFi saved) Config portal active
5. **"WIFI OK"** - WiFi connected successfully
6. **IP address display** - Shows "IP:xxx.xxx." (top) and "xxx.xxx" (bottom) for 2.5 seconds
7. **"NTP"** - Syncing time with NTP server
8. **"TIME OK"** - Time synchronized
9. **"READY"** - Setup complete, starting clock display

## Installation

### Using PlatformIO (Recommended)

1. Install [PlatformIO](https://platformio.org/install)
2. Clone or download this project
3. Open the project folder in VS Code with PlatformIO
4. Build and upload:
   ```bash
   pio run -t upload
   ```
5. Monitor serial output:
   ```bash
   pio device monitor
   ```

### Using Arduino IDE

1. Install [ESP32 board support](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
2. Install required libraries:
   - TFT_eSPI by Bodmer
   - WiFiManager by tzapu
   - Adafruit BME280 Library (if using BME280 sensor)
   - Adafruit SHT31 Library (if using SHT3X sensor)
   - Adafruit HTU21DF Library (if using HTU21D sensor)
   - Adafruit Unified Sensor
3. Copy `User_Setup.h` to your TFT_eSPI library folder (replace existing)
4. Open `cyd_tft_clock.cpp` and rename to `cyd_tft_clock.ino`
5. Configure your sensor type (see Sensor Configuration section)
6. Compile and upload

## WiFi Configuration

### Three Ways to Configure WiFi:

#### 1. **BOOT Button Method** (Recommended for first-time setup)
1. **Hold the BOOT button** (GPIO 0) while powering on the device
2. **Keep holding for 3 seconds** until LED turns RED
3. **Release button** - WiFi credentials will be cleared
4. Device enters config portal mode
5. **Connect to "CYD_Clock_Setup"** WiFi network
6. **Open 192.168.4.1** in a browser
7. **Enter your WiFi credentials**
8. Clock will connect and sync time automatically

#### 2. **Web Interface Method**
1. Access the web interface at `http://<IP_ADDRESS>`
2. Scroll to **System** section
3. Click **"Reset WiFi"** button
4. Device restarts in config portal mode
5. Connect to "CYD_Clock_Setup" network and reconfigure

#### 3. **Serial Monitor Method** (for development)
1. Upload code with fresh ESP32 or erased flash
2. WiFiManager automatically starts config portal
3. Connect to "CYD_Clock_Setup" network
4. Configure WiFi credentials

## Web Interface

Once connected, access the web interface at the clock's IP address (displayed on startup):

- **Live Clock Display**: Real-time time and date
- **TFT Display Mirror**: Canvas-based simulation of the LED matrix with live updates
- **Environment Data**: Temperature, humidity, pressure (if BME280 sensor connected)
- **Settings**:
  - Toggle °C/°F temperature units
  - Toggle 12/24 hour format
  - Toggle leading zero for hours < 10
  - Select timezone (87 global options organized by region)
- **Display Customization**:
  - Toggle display style (Blocks/Realistic LEDs)
  - Choose LED color (8 options)
  - Choose surround color (8 options including "Match LED")
  - Adjust LED size (4-12 pixels)
  - Adjust LED spacing (0-3 pixels)
  - Set mode switch interval (1-60 seconds)
- **System Information**:
  - Board model (ESP32 CYD)
  - Sensor type and status
  - IP address
  - Uptime
  - Free heap memory
  - WiFi reset button
- **Footer**:
  - Links to GitHub repository and Bluesky profile
  - Attribution and credits

## Serial Monitor Diagnostics

Connect at 115200 baud to see detailed diagnostics:

### Startup Information:
- Device banner and version
- BOOT button status (WiFi reset detection)
- TFT display initialization
- WiFi connection details (SSID, IP, Gateway, DNS, RSSI, Mode)
- Sensor detection status
- NTP sync results
- Web server startup confirmation

### Runtime Monitoring (every 60 seconds):
- Current time and date
- Temperature and humidity
- Free heap memory
- **WiFi status**: Connection state, IP address, signal strength (RSSI)

### Mode-Specific Output (every second):
- **Mode 0**: Displays time, AM/PM (if 12-hour), temperature, and humidity
- **Mode 1**: Displays large time with seconds
- **Mode 2**: Displays time with seconds and date

### Settings Changes (on-demand):
- Detailed output shown only when user modifies settings via web interface
- Includes: temperature units, time format, leading zero, timezone, display style, LED colors, LED size/spacing, mode switch interval

### Network Diagnostics:
The Serial Monitor displays complete network information on connection:
```
=== WiFi Connected ===
SSID: YourNetworkName
IP Address: xxx.xxx.xxx.xxx
Gateway: xxx.xxx.xxx.xxx
Subnet Mask: xxx.xxx.xxx.xxx
DNS: xxx.xxx.xxx.xxx
Signal Strength (RSSI): -45 dBm
WiFi Mode: STA
```

## Sensor Configuration

### Choosing Your Sensor

The project supports three types of environmental sensors. You must configure which sensor you're using before compiling.

#### Option 1: BME280 (Temperature, Humidity, and Pressure)
- Measures temperature, relative humidity, and barometric pressure
- I2C addresses: 0x76 or 0x77 (auto-detected)
- Perfect for weather monitoring with pressure data

#### Option 2: SHT3X (Temperature and Humidity)
- Measures temperature and relative humidity (no pressure)
- I2C addresses: 0x44 or 0x45 (auto-detected)
- More accurate temperature/humidity readings than BME280

#### Option 3: HTU21D (Temperature and Humidity)
- Measures temperature and relative humidity (no pressure)
- I2C address: 0x40 (fixed)
- Reliable and widely available sensor

### Configuring the Sensor Type

Edit [src/cyd_tft_clock.cpp](src/cyd_tft_clock.cpp) around line 84:

```cpp
// Choose your sensor type by uncommenting ONE of the following:
#define USE_BME280        // BME280: Temperature, Humidity, Pressure sensor
// #define USE_SHT3X      // SHT3X: Temperature and Humidity sensor (no pressure)
// #define USE_HTU21D     // HTU21D: Temperature and Humidity sensor (no pressure)
```

**For BME280**: Leave `#define USE_BME280` uncommented (default)
**For SHT3X**: Comment out BME280 and uncomment `#define USE_SHT3X`
**For HTU21D**: Comment out BME280 and uncomment `#define USE_HTU21D`

### Connecting the Sensor

Connect to the extended GPIO connector (CN1):

| Sensor Pin | CYD Pin | Notes |
|------------|---------|-------|
| VCC | 3.3V | **Important: Use 3.3V, NOT 5V** |
| GND | GND | Ground |
| SDA | GPIO 27 | I2C Data |
| SCL | GPIO 22 | I2C Clock |

**Important Notes:**
- All sensors use the same I2C pins (GPIO 27 and 22)
- Only connect **one sensor at a time**
- Always use **3.3V power** - 5V may damage the sensors
- The device operates normally without a sensor (displays "NO SENSOR")
- Sensor type is shown in the web interface
- **Sensor I2C Addresses:**
  - BME280: 0x76 or 0x77 (auto-detected)
  - SHT3X: 0x44 or 0x45 (auto-detected)
  - HTU21D: 0x40 (fixed address)

## RGB LED Status Indicators

### During Startup:
| Color | Meaning |
|-------|---------|
| Blue flash | Device startup |
| Yellow (steady) | BOOT button detected, waiting for 3-second hold |
| Red (steady) | WiFi reset confirmed (after holding BOOT for 3s) |
| Blue (steady) | Connecting to WiFi |
| Purple (steady) | WiFi config portal mode active |
| Green flash | Success (WiFi connected, NTP synced, sensor found) |
| Yellow flash | Sensor not found |
| Red flash | Error/failure |

### During Operation:
| Event | LED Indicator |
|-------|---------------|
| NTP sync success | Green flash |
| NTP sync failure | Red flash |
| WiFi disconnected | Red flash + auto-reconnect |

## Project Structure

```
CYD_TFT_RetroClock/
├── src/
│   └── cyd_tft_clock.cpp    # Main application code with WiFi config
├── include/
│   ├── User_Setup.h         # TFT_eSPI display configuration for CYD
│   ├── fonts.h              # LED matrix font definitions (3x7, 5x8, 5x16, etc.)
│   └── timezones.h          # 88 global timezone POSIX strings
├── images/
│   └── Reference_CYD.jpeg   # ESP32 CYD board hardware reference image
├── platformio.ini           # PlatformIO build configuration
├── CHANGELOG.md             # Project changelog
├── README.md                # This documentation file
└── .gitignore               # Git ignore rules
```

## Differences from ESP8266 Version

| Feature | ESP8266 | ESP32 CYD |
|---------|---------|-----------|
| WiFi Library | ESP8266WiFi.h | WiFi.h |
| Web Server | ESP8266WebServer.h | WebServer.h |
| TFT Pins | External wiring | Built-in (HSPI) |
| Display Size | 320×240 | 320×240 (same) |
| Backlight | D8 | GPIO 21 |
| I2C SDA | D4 | GPIO 27 |
| I2C SCL | D3 | GPIO 22 |
| LED Size | 10px | 9px default (user-adjustable 4-12px) |
| RGB LED | None | Built-in (GPIO 4, 16, 17) |
| Boot Button | None | GPIO 0 (WiFi reset feature) |
| Time Config | configTime() | configTzTime() (ESP32-specific) |
| Loop Delay | 100ms | 1ms (faster, more responsive) |
| WiFi Monitor | No | Yes (every 10s with auto-reconnect) |
| IP Display | No | Yes (shown on TFT at startup) |
| Diagnostics | Basic | Comprehensive (Serial + Network) |

## Troubleshooting

### Display is blank
- Check User_Setup.h is properly configured
- Verify backlight pin (GPIO 21) is set HIGH
- Try different rotation values (0-3)

### Colors are wrong (red/blue swapped)
- Uncomment `#define TFT_RGB_ORDER TFT_BGR` in User_Setup.h

### Display shows artifacts
- Reduce SPI frequency to 27000000
- Check for loose connections

### WiFi won't connect
- **Method 1**: Hold BOOT button for 3 seconds during power-up to reset WiFi
- **Method 2**: Access `/reset` endpoint in web interface
- **Method 3**: Open Serial Monitor (115200 baud) to see connection diagnostics
- Check router isn't using client isolation
- Verify firewall isn't blocking port 80

### Web server not accessible
- Check Serial Monitor for IP address and WiFi status
- Verify device shows "WiFi Mode: STA" in diagnostics
- Ensure your computer is on the same WiFi network
- Try accessing from different device/browser
- Check router firewall settings

### Sensor not detected
- Verify I2C wiring to GPIO 27 (SDA) and GPIO 22 (SCL)
- Check that the correct sensor type is defined in [src/cyd_tft_clock.cpp](src/cyd_tft_clock.cpp)
- **I2C Addresses:**
  - BME280: 0x76 or 0x77
  - SHT3X: 0x44 or 0x45
  - HTU21D: 0x40
- Ensure 3.3V power is connected (not 5V)
- Check Serial Monitor for sensor detection messages

### Device keeps restarting
- Check Serial Monitor for error messages
- Verify stable power supply (ESP32 needs good 5V/1A minimum)
- WiFi credentials may be incorrect - reset WiFi config

## License

MIT License - See LICENSE file for details

## Acknowledgements

This project is based on the original **ESP8266 TFT LED Matrix Clock** created by **@cbm80amiga (Pawel A.)**

- Original project: [YouTube Video](https://www.youtube.com/watch?v=2wJOdi0xzas)
- Original code repository: https://drive.google.com/drive/folders/1dfWRP2fgwyA4KJZyiFvkcBOC8FUKdx53 

This version has been significantly refactored and enhanced for the ESP32 Cheap Yellow Display (CYD) board with additional features including WiFiManager, multiple sensor support, comprehensive web interface, and more.

### Additional Credits

- **ESP32 CYD Community**: [ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- **TFT_eSPI Library**: by Bodmer
- **WiFiManager Library**: by tzapu
- **Adafruit Sensor Libraries**: BME280, SHT31, HTU21DF libraries

### Connect

- **GitHub**: [anthonyjclarke/CYD_TFT_RetroClock](https://github.com/anthonyjclarke/CYD_TFT_RetroClock)
- **Bluesky**: [@anthonyjclarke.bsky.social](https://bsky.app/profile/anthonyjclarke.bsky.social)

Built with ❤️ by Anthony Clarke
