# Quick Start Guide - ESP32 CYD TFT Matrix Clock

## 🚀 Get Up and Running in 10 Minutes

This guide will get your ESP32 CYD LED Matrix Clock working quickly. The CYD (Cheap Yellow Display) is an all-in-one board - no wiring required!

For detailed information, see [README.md](README.md).

## 📦 What You Need

### Hardware (All-in-One Board!)
- **ESP32-2432S028R (CYD)** board (~$15)
  - Includes built-in 2.8" ILI9341 TFT display (320×240)
  - Integrated ESP32 microcontroller
  - Built-in RGB LED
  - Built-in BOOT button
  - USB-C power connector
- **USB-C cable** for programming and power

### Optional: Environmental Sensor
- **One of these** (choose based on your needs):
  - **BME280** - Temperature, Humidity, Pressure (I2C 0x76/0x77)
  - **SHT3X** - Temperature, Humidity - high accuracy (I2C 0x44/0x45)
  - **HTU21D** - Temperature, Humidity - reliable (I2C 0x40)
- **4 jumper wires** (Female-to-Female) if using sensor

## 🔌 Wiring

### Basic Setup (Clock Only)
**NO WIRING REQUIRED!** The ESP32 CYD has everything built-in.

Just plug in the USB-C cable and you're ready to go!

### Full Setup (With Environmental Sensor)

If you want temperature/humidity readings, connect **ONE** sensor to the CN1 extended GPIO connector:

```
Sensor → CYD CN1 Connector
────────────────────────────
VCC    → 3.3V  (⚠️ NOT 5V!)
GND    → GND
SDA    → GPIO 27
SCL    → GPIO 22
```

**⚠️ IMPORTANT:** Use 3.3V power, NOT 5V! 5V will damage the sensor.

## 💻 Software Setup

### 1. Install PlatformIO

**Option A: VS Code (Recommended)**
1. Install [VS Code](https://code.visualstudio.com/)
2. Install "PlatformIO IDE" extension
3. Restart VS Code

**Option B: Command Line**
```bash
pip install platformio
```

### 2. Download the Project

Clone or download this repository to your computer.

### 3. Configure Your Sensor (Optional)

If you're using an environmental sensor, edit `src/cyd_tft_clock.cpp` around line 84-91:

```cpp
// Choose your sensor type by uncommenting ONE of the following:
#define USE_BME280        // BME280: Temp, Humidity, Pressure (default)
// #define USE_SHT3X      // SHT3X: Temp, Humidity (high accuracy)
// #define USE_HTU21D     // HTU21D: Temp, Humidity (reliable)
```

**Uncomment only ONE sensor type** based on what you have connected.

### 4. Upload to ESP32 CYD

**Using VS Code:**
- Click "Upload" button (→ icon) in PlatformIO toolbar
- Or press `Ctrl+Alt+U` (Windows/Linux) / `Cmd+Alt+U` (Mac)

**Using Command Line:**
```bash
pio run -t upload
```

**Note:** The board should be detected automatically. If not, check `platformio.ini` for the correct upload port.

## 📱 First Time Setup

### 1. Power On
Watch the TFT display show these startup messages:
- **"INIT"** - Initializing hardware
- **"WIFI"** - Starting WiFi configuration

### 2. Connect to WiFi

The clock uses WiFiManager for easy setup:

**Method 1: BOOT Button Reset (Recommended)**
1. **Hold the BOOT button** (on the CYD board) while powering on
2. **Keep holding for 3 seconds** until RGB LED turns **RED**
3. Release button
4. Look for WiFi network: **"CYD_Clock_Setup"**
5. Connect with phone/computer
6. Browser opens automatically (or go to http://192.168.4.1)
7. Select your WiFi network
8. Enter password
9. Click "Save"

**Method 2: First Boot (if no WiFi configured)**
1. WiFi config portal starts automatically
2. Connect to **"CYD_Clock_Setup"** WiFi network
3. Follow steps 6-9 above

### 3. Watch the Startup Sequence

After WiFi connection, you'll see:
- `WIFI OK` → WiFi connected successfully
- `IP:192.x.` → Your IP address (shown for 2.5 seconds)
- `    x.x`
- `NTP` → Syncing time with NTP server
- `TIME OK` → Time synchronized
- `READY` → System ready
- **Clock starts showing time!**

## 🌐 Access Web Interface

### Find Your IP Address

**Method 1: Look at TFT Display**
- IP address is shown on TFT for 2.5 seconds during startup
- Example: "IP:192.168." (top), "1.123" (bottom)

**Method 2: Serial Monitor**
```bash
pio device monitor
```
Look for: `IP Address: 192.x.x.x`

**Method 3: Router**
- Check DHCP client list
- Look for device named "ESP_xxxxxx"

### Open Web Interface

Go to: `http://192.x.x.x` (use your IP from above)

You'll see:
- **Live Clock Display**: Current time and date
- **TFT Display Mirror**: Real-time simulation of the LED matrix
- **Environment Data**: Temperature, humidity, pressure (if sensor connected)
- **Settings**: Timezone, temperature unit, display style, LED colors

## ⚙️ Basic Configuration

### Set Your Timezone

1. In web interface, find **"Timezone & Time Format"** section
2. Select your region and city from organized dropdown
3. Time updates automatically
4. No page reload needed!

**Organized by Region:**
- Australia & Oceania
- North America
- South America
- Western Europe
- Northern Europe
- Central & Eastern Europe
- Middle East
- South Asia
- Southeast Asia
- East Asia
- Central Asia
- Caucasus
- Africa

**Default:** Sydney, Australia

### Toggle Temperature Unit

Click **"Toggle °C/°F"** button to switch between Celsius and Fahrenheit.

### Toggle Time Format

Click **"Toggle 12/24 Hour"** button to switch between 12-hour (with AM/PM) and 24-hour format.

### Toggle Leading Zero

Click **"Toggle Leading Zero"** to show/hide leading zero for hours < 10:
- **OFF**: "1:23 AM" or "9:45"
- **ON**: "01:23 AM" or "09:45"

### Set Mode Switch Interval

Use the **Mode Switch Interval slider** (1-60 seconds):
- Adjust how long each display mode shows before switching
- Default: 5 seconds
- Range: 1-60 seconds
- Located in "Display Customization" section below LED Spacing

## 🎨 Customize Appearance

### Change Display Style

In web interface, find **"Display Style"** section:
- **Default (Blocks)**: Solid square blocks
- **Realistic (LEDs)**: Circular LEDs with housing

Click **"Toggle Style"** to switch.

### Change LED Color

Select from 8 color presets:
- Red (default)
- Green
- Blue
- Yellow
- Cyan
- Magenta
- White
- Orange

### Adjust LED Size

Use the **LED Size slider** (4-12 pixels):
- Smaller (4-6px): More modern, fits more content
- Default (9px): Balanced look
- Larger (10-12px): More retro/vintage feel

**💡 Tip:** If seconds are truncated in 24-hour mode, reduce LED Size or adjust Spacing.

### Adjust LED Spacing

Use the **LED Spacing slider** (0-3 pixels):
- 0px: Tightest spacing
- 1px: Default, balanced
- 2-3px: More authentic vintage spacing

### Change Surround Color (Realistic Style Only)

Select from 7 surround color options:
- White
- Light Gray
- Dark Gray (default - authentic MAX7219 look)
- Red
- Green
- Blue
- Yellow
- Match LED Color

## 📊 Display Modes

The clock cycles through 3 display modes (interval configurable via web interface, default: 5 seconds):

### Mode 0: Time + Temperature
```
Top Row:    10:24 AM  (12-hour) or  22:24  (24-hour)
Bottom Row: T23C H45%

Note: Seconds NOT displayed in this mode
      AM/PM shown in 12-hour mode
      Colon FLASHES every second
      All text uses font3x7 (7-pixel height)
```

### Mode 1: Large Time
```
Full Display: Large 16-pixel time

      10:24:35
   (with small seconds)
      Colon FLASHES every second
```

### Mode 2: Time + Date
```
Top Row:    10:24:35
Bottom Row: 05/01/26 (DD/MM/YY format)

      Colon FLASHES every second
```

## 🔧 Troubleshooting

### Display is Blank
- ✅ Check USB-C power connection (needs 5V, 1A minimum)
- ✅ Press RESET button on CYD board
- ✅ Check serial monitor for errors: `pio device monitor`
- ✅ Verify backlight is on (GPIO 21)

### Display Shows Wrong Colors (Red/Blue Swapped)
- ✅ Edit `include/User_Setup.h`
- ✅ Uncomment: `#define TFT_RGB_ORDER TFT_BGR`
- ✅ Re-upload code

### WiFi Not Connecting
- ✅ Hold BOOT button for 3 seconds during power-up (LED turns RED)
- ✅ Connect to "CYD_Clock_Setup" network
- ✅ Check 2.4GHz network (ESP32 doesn't support 5GHz)
- ✅ Check serial monitor for diagnostics

### Time Not Showing
- ✅ Verify WiFi connection (check serial monitor)
- ✅ Wait 20 seconds for NTP sync
- ✅ Check timezone is set correctly
- ✅ Verify firewall isn't blocking NTP (port 123)

### Sensor Not Working
- ✅ Check I2C wiring: GPIO 27 (SDA), GPIO 22 (SCL)
- ✅ Verify 3.3V power (NOT 5V!)
- ✅ Check correct sensor type defined in code
- ✅ Try different I2C address:
  - BME280: 0x76 or 0x77
  - SHT3X: 0x44 or 0x45
  - HTU21D: 0x40 (fixed)
- ✅ Check serial monitor: Look for "BME280 OK" or similar message

### Web Interface Not Accessible
- ✅ Verify device shows "WiFi Mode: STA" in serial monitor
- ✅ Ensure computer is on same WiFi network
- ✅ Try different browser
- ✅ Check router firewall settings
- ✅ Check router isn't using client isolation

### RGB LED Shows Red/Error
- ✅ Check serial monitor for specific error messages
- ✅ WiFi connection may have failed
- ✅ NTP sync may have failed
- ✅ Sensor initialization may have failed (yellow flash = sensor not found)

## 🎯 RGB LED Status Guide

### During Startup:
| Color          | Meaning                                              |
|----------------|------------------------------------------------------|
| Blue flash     | Device starting up                                   |
| Yellow (steady)| BOOT button detected, waiting for 3-second hold      |
| Red (steady)   | WiFi reset confirmed (after 3-second button hold)    |
| Blue (steady)  | Connecting to WiFi                                   |
| Purple (steady)| Config portal active (connect to "CYD_Clock_Setup")  |
| Green flash    | Success (WiFi connected, NTP synced, sensor found)   |
| Yellow flash   | Sensor not found (clock still works without sensor)  |
| Red flash      | Error (check serial monitor)                         |

### During Operation:
| Event              | LED Indicator              |
|--------------------|----------------------------|
| NTP sync success   | Green flash                |
| NTP sync failure   | Red flash                  |
| WiFi disconnected  | Red flash + auto-reconnect |

## 📚 Serial Monitor Diagnostics

The serial monitor (115200 baud) shows detailed information:

```bash
pio device monitor
```

### Startup Information:
- Device banner and version
- BOOT button status
- TFT display initialization
- WiFi connection details (SSID, IP, Gateway, DNS, RSSI, Mode)
- Sensor detection status
- NTP sync results
- Web server startup

### Runtime Monitoring (every 60 seconds):
- Current time and date
- Temperature and humidity (if sensor connected)
- Free heap memory
- WiFi status with signal strength

### Mode-Specific Output (every second):
- Mode 0: Time, AM/PM (if 12-hour), temperature, humidity
- Mode 1: Large time with seconds
- Mode 2: Time with seconds and date

### Settings Changes (on-demand):
- Detailed output only when user modifies settings via web interface
- Shows: temperature units, time format, leading zero, timezone, display style, LED colors, sizes, mode interval

## 🎉 Success Checklist

- [ ] TFT display shows "INIT" on power up
- [ ] RGB LED shows blue flash at startup
- [ ] WiFi connected (display shows "WIFI OK")
- [ ] IP address displayed on TFT (2.5 seconds)
- [ ] Time synchronized (display shows "TIME OK")
- [ ] Clock displays current time
- [ ] Web interface accessible
- [ ] Can change timezone in web interface
- [ ] Display cycles through 3 modes (default: 5 seconds, configurable 1-60s)
- [ ] Colon flashes every second in all modes
- [ ] Can adjust mode switch interval via slider
- [ ] (Optional) Temperature/humidity showing
- [ ] (Optional) Sensor type shown in web interface

## 📋 Next Steps

Once basic setup works:

1. **Customize Appearance**
   - Try different LED colors
   - Adjust LED size and spacing
   - Switch between Default and Realistic styles
   - Try different surround colors

2. **Add Environmental Sensor**
   - Connect BME280, SHT3X, or HTU21D
   - Configure sensor type in code
   - View temperature/humidity in Mode 0

3. **Explore Web Interface**
   - View live TFT display mirror
   - Check system information
   - Monitor WiFi signal strength
   - Adjust time format and leading zero

4. **Optimize Display**
   - Adjust LED size if seconds are truncated
   - Fine-tune spacing for your preference
   - Try different color themes

## 📚 More Information

- **Full Documentation**: [README.md](README.md)
- **Comparison with LED Matrix**: [COMPARISON.md](COMPARISON.md)
- **Detailed Changes**: [CHANGELOG.md](CHANGELOG.md)
- **Pin Configuration**: See README.md "Pin Configuration" section
- **Sensor Configuration**: See README.md "Sensor Configuration" section

## 🆘 Getting Help

### Check Serial Monitor
The serial monitor provides detailed diagnostics:
```bash
pio device monitor
```

Press `Ctrl+C` to exit monitor.

### Common Issues

**"No module named 'serial'"**
```bash
pip install pyserial
```

**"Upload failed"**
- Check USB-C cable (needs data capability, not just power)
- Try different USB port
- Check device is detected: `pio device list`

**"Permission denied"**
- On Linux/Mac, may need udev rules or permissions
- Try: `sudo chmod 666 /dev/ttyUSB0` (adjust device name)

## ✅ You're Done!

Your ESP32 CYD LED Matrix Clock is now running!

Enjoy your retro-style clock with modern ESP32 technology and all-in-one convenience.

---

**Remember:** The ESP32 CYD provides everything in one board - display, microcontroller, RGB LED, and boot button. No wiring, no soldering, just plug and play! 🎨⏰

**Need WiFi Reset?** Hold BOOT button for 3 seconds during power-up (LED turns RED).

**Web Interface:** Access all settings at `http://YOUR_IP_ADDRESS`

**Have fun customizing your clock!** 🚀
