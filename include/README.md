# ESP32 CYD TFT Matrix Clock

A retro LED matrix clock simulator running on the ESP32 Cheap Yellow Display (CYD) board.

## Hardware

### Board: ESP32-2432S028R (Cheap Yellow Display)

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
| **I2C (Extended GPIO)** | | For BME280 sensor |
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
- **88 Timezones**: Comprehensive global timezone support
- **BME280 Sensor**: Optional temperature, humidity, and pressure readings
- **Web Interface**: Modern responsive control panel with live display mirror
- **Display Styles**: Default blocks or realistic circular LEDs
- **8 LED Colors**: Red, Green, Blue, Yellow, Cyan, Magenta, White, Orange
- **RGB LED Status**: Visual feedback during startup and operation
- **Serial Diagnostics**: Real-time monitoring at 115200 baud

## Display Modes

The clock cycles through three display modes every 5 seconds:

1. **Time + Temperature**: Time on top row, temp/humidity on bottom
2. **Large Time**: Full 16-pixel tall time display
3. **Time + Date**: Time on top, date on bottom

## Startup Sequence

The TFT display shows the following messages during boot:

1. **"INIT"** - Initializing hardware
2. **"RESET"** + **"WIFI"** - (Only if BOOT button held) WiFi reset in progress
3. **"WIFI"** - Connecting to WiFi
4. **"SETUP AP"** - (If no WiFi saved) Config portal active
5. **"WIFI OK"** - WiFi connected successfully
6. **IP address display** - Shows "IP:192.168." (top) and "1.123" (bottom) for 2.5 seconds
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
   - Adafruit BME280 Library
   - Adafruit Unified Sensor
3. Copy `User_Setup.h` to your TFT_eSPI library folder (replace existing)
4. Open `cyd_tft_clock.cpp` and rename to `cyd_tft_clock.ino`
5. Compile and upload

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
- **TFT Display Mirror**: Canvas-based simulation of the LED matrix
- **Environment Data**: Temperature, humidity, pressure (if sensor connected)
- **Settings**:
  - Toggle °C/°F
  - Toggle 12/24 hour format
  - Change timezone (88 options)
  - Change display style
  - Change LED color
  - Change surround color
- **System Information**:
  - Board model
  - IP address
  - Uptime
  - Free heap memory
  - WiFi reset button

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

### Runtime Monitoring (every 10 seconds):
- Current time and date
- Temperature and humidity
- Free heap memory
- **WiFi status**: Connection state, IP address, signal strength (RSSI)

### Network Diagnostics:
The Serial Monitor displays complete network information on connection:
```
=== WiFi Connected ===
SSID: YourNetworkName
IP Address: 192.168.1.123
Gateway: 192.168.1.1
Subnet Mask: 255.255.255.0
DNS: 192.168.1.1
Signal Strength (RSSI): -45 dBm
WiFi Mode: STA
```

## Connecting a BME280 Sensor

Connect to the extended GPIO connector (CN1):

| BME280 | CYD Pin |
|--------|---------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 27 |
| SCL | GPIO 22 |

The sensor auto-detects at addresses 0x76 or 0x77.

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
│   ├── timezones.h          # 88 global timezone POSIX strings
│   └── README.md            # This documentation file
├── platformio.ini           # PlatformIO build configuration
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
| LED Size | 10px | 10px (optimized for 32×16 matrix) |
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
- Check sensor address (0x76 or 0x77)
- Ensure 3.3V power is connected (not 5V)

### Device keeps restarting
- Check Serial Monitor for error messages
- Verify stable power supply (ESP32 needs good 5V/1A minimum)
- WiFi credentials may be incorrect - reset WiFi config

## License

MIT License - See LICENSE file for details

## Credits

- Original LED Matrix Clock by Anthony Clarke (AJC & Co)
- CYD Community: [ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- TFT_eSPI Library by Bodmer
