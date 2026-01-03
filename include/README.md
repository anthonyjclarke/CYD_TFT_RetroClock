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
- **NTP Time Sync**: Automatic time synchronization with DST support
- **88 Timezones**: Comprehensive global timezone support
- **BME280 Sensor**: Optional temperature, humidity, and pressure readings
- **Web Interface**: Modern responsive control panel with live display mirror
- **Display Styles**: Default blocks or realistic circular LEDs
- **8 LED Colors**: Red, Green, Blue, Yellow, Cyan, Magenta, White, Orange
- **RGB LED Status**: Visual feedback during startup and operation

## Display Modes

The clock cycles through three display modes every 5 seconds:

1. **Time + Temperature**: Time on top row, temp/humidity on bottom
2. **Large Time**: Full 16-pixel tall time display
3. **Time + Date**: Time on top, date on bottom

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

## First Boot

1. The clock starts in WiFi configuration mode
2. Connect to the "CYD_Clock_Setup" WiFi network
3. Open 192.168.4.1 in a browser
4. Enter your WiFi credentials
5. The clock will connect and sync time automatically

## Web Interface

Once connected, access the web interface at the clock's IP address:

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

| Color | Meaning |
|-------|---------|
| Blue flash | Startup |
| Purple (steady) | WiFi config mode |
| Green flash | Success (WiFi connected, NTP synced, sensor found) |
| Yellow flash | Sensor not found |
| Red flash | Error/failure |

## Project Structure

```
cyd_clock/
├── cyd_tft_clock.cpp    # Main application code
├── User_Setup.h         # TFT_eSPI display configuration
├── fonts.h              # LED matrix font definitions
├── timezones.h          # 88 global timezone definitions
├── platformio.ini       # PlatformIO build configuration
└── README.md            # This file
```

## Differences from ESP8266 Version

| Feature | ESP8266 | ESP32 CYD |
|---------|---------|-----------|
| WiFi Library | ESP8266WiFi.h | WiFi.h |
| Web Server | ESP8266WebServer.h | WebServer.h |
| TFT Pins | External wiring | Built-in |
| Backlight | D8 | GPIO 21 |
| I2C SDA | D4 | GPIO 27 |
| I2C SCL | D3 | GPIO 22 |
| LED Size | 10px | 14px (larger display) |
| RGB LED | None | Built-in (GPIO 4, 16, 17) |
| Time Config | configTime() | configTzTime() |
| Loop Delay | 100ms | 10ms |

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
- Press and hold reset for 5 seconds to clear WiFi settings
- Access `/reset` endpoint to clear credentials

### Sensor not detected
- Verify I2C wiring to GPIO 27 (SDA) and GPIO 22 (SCL)
- Check sensor address (0x76 or 0x77)

## License

MIT License - See LICENSE file for details

## Credits

- Original LED Matrix Clock by Anthony Clarke (AJC & Co)
- CYD Community: [ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
- TFT_eSPI Library by Bodmer
