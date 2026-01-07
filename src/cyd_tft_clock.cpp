/*
 * ESP32 CYD TFT Matrix Clock
 * Author: Refactored for ESP32 CYD by Anthony Clarke
 * Board: ESP32-2432S028R (Cheap Yellow Display)
 * Display: Built-in 2.8" ILI9341 320x240 TFT
 *
 * ======================== ACKNOWLEDGEMENTS ========================
 * This project is based on the original ESP8266 TFT LED Matrix Clock
 * created by @cbm80amiga (Pawel A.)
 * Original project: https://www.youtube.com/watch?v=2wJOdi0xzas
 * Code: https://drive.google.com/drive/folders/1dfWRP2fgwyA4KJZyiFvkcBOC8FUKdx53 
 * Author's GitHub: https://github.com/cbm80amiga
 * 
 * Then updated to include WiFiManager, NTP sync, I2C sensor support
 * https://github.com/anthonyjclarke/ESP_LEDMatrix_32x16_NTP_Clock
 * This has been deprecated in favor of this ESP32 CYD version.
 * 
 * ======================== PROJECT DESCRIPTION ========================
 * This version has been significantly refactored and enhanced to run
 * on the ESP32 Cheap Yellow Display (CYD) board with additional features
 * including WiFiManager, multiple sensor support, web interface, and more.
 *
 * ======================== WiFi CONFIGURATION ========================
 * Three ways to reset/configure WiFi:
 *
 * 1. BOOT BUTTON METHOD (Recommended for first-time setup):
 *    - Hold the BOOT button (GPIO 0) during power-up
 *    - Keep holding for 3 seconds until LED turns RED
 *    - Release button - WiFi credentials will be cleared
 *    - Device will enter config portal mode
 *    - Connect to "CYD_Clock_Setup" WiFi network
 *    - Configure your WiFi credentials in the web portal
 *
 * 2. WEB INTERFACE METHOD:
 *    - Access the web interface at http://<IP_ADDRESS>
 *    - Scroll to "System" section
 *    - Click "Reset WiFi" button
 *    - Device will restart in config portal mode
 *
 * 3. SERIAL MONITOR METHOD (for development):
 *    - Upload code with fresh ESP32 (or with erased flash)
 *    - WiFiManager will automatically start config portal
 *    - Connect to "CYD_Clock_Setup" network
 *
 * LED Indicators during WiFi setup:
 *    - YELLOW: BOOT button detected, waiting for 3-second hold
 *    - RED: WiFi reset confirmed
 *    - BLUE: Connecting to WiFi
 *    - PURPLE: Config portal active (AP mode)
 *    - GREEN: WiFi connected successfully
 * 
 * * ======================== To Do ========================
 *   - Enable Remote upload of firmware via web interface
 *   - Add support for additional display modes
 *   - Implement more advanced time zone handling
 *   - Add external weather API support
 *   - Make code portable to other ESP32 and TFT boards
 *
 * ======================== FEATURES ========================
 * - Simulates 4x2 MAX7219 LED matrix appearance on TFT display
 * - WiFiManager for easy WiFi setup (no hardcoded credentials)
 * - BME280 I2C temperature/pressure/humidity sensor (optional)
 * - Automatic NTP time synchronization with DST support
 * - Modern responsive web interface with live updates
 * - Realistic LED rendering with customizable colors
 * - Multiple timezone support with POSIX TZ strings
 * - Two display styles: Default (blocks) and Realistic (circular LEDs)
 * - RGB LED indicator on CYD board for status
 *
 * ======================== CYD HARDWARE ========================
 * TFT Display: Built-in 2.8" ILI9341 320x240 (HSPI)
 * Touchscreen: XPT2046 resistive (VSPI) - not used in this project
 * RGB LED: GPIO 4 (Red), GPIO 16 (Green), GPIO 17 (Blue) - Active LOW
 * I2C: GPIO 27 (SDA), GPIO 22 (SCL) - via extended connector
 * Backlight: GPIO 21
 * SD Card: Available but not used
 */

#include "Arduino.h"

// ======================== SENSOR CONFIGURATION ========================
// Choose your sensor type by uncommenting ONE of the following:
// #define USE_BME280        // BME280: Temperature, Humidity, Pressure sensor
// #define USE_SHT3X      // SHT3X: Temperature and Humidity sensor (no pressure)
#define USE_HTU21D     // HTU21D: Temperature and Humidity sensor (no pressure)

// ======================== LIBRARIES ========================
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#ifdef USE_BME280
  #include <Adafruit_BME280.h>
#endif
#ifdef USE_SHT3X
  #include <Adafruit_SHT31.h>
#endif
#ifdef USE_HTU21D
  #include <Adafruit_HTU21DF.h>
#endif
#include <time.h>
#include <TFT_eSPI.h>  // Hardware-specific library with optimized performance
#include <DNSServer.h> // Required for WiFiManager on ESP32

// ======================== PIN DEFINITIONS ========================
// TFT Display (built-in on CYD, configured in User_Setup.h)
// MOSI: GPIO 13, MISO: GPIO 12, CLK: GPIO 14
// CS: GPIO 15, DC: GPIO 2, RST: -1

// Backlight control
#define TFT_BL_PIN    21    // TFT Backlight control

// RGB LED (active low - inverted logic)
#define LED_R_PIN      4    // Red LED
#define LED_G_PIN     16    // Green LED
#define LED_B_PIN     17    // Blue LED

// I2C for sensor (using extended GPIO connector CN1)
#define SDA_PIN       27    // I2C Data
#define SCL_PIN       22    // I2C Clock

// Boot button (built-in on CYD board)
#define BOOT_BTN_PIN   0    // Boot button (active LOW)

// ======================== DISPLAY CONFIGURATION ========================
// Virtual LED Matrix dimensions
#define NUM_MAX           8      // Simulated number of 8x8 LED matrices (2 rows × 4 columns)
#define MATRIX_WIDTH      8      // Width of each simulated matrix
#define MATRIX_HEIGHT     8      // Height of each simulated matrix
#define LINE_WIDTH        32     // Display width in pixels (4 matrices wide)
#define DISPLAY_ROWS      2      // Number of rows of matrices
#define TOTAL_WIDTH       32     // Total width: 32 pixels
#define TOTAL_HEIGHT      16     // Total height: 16 pixels (2 rows of 8)

// CYD has a 320x240 display - LED size must fit 32 pixels across 320px width
// These are now variables that can be changed via web interface
// NOTE: Reducing ledSize allows more content to fit on screen (e.g., full seconds in 24-hour mode)
//       Smaller LED sizes (4-8px) prevent truncation; larger sizes (10-12px) may clip seconds
int ledSize = 9;            // Size of each simulated LED pixel (default: 9 * 32 = 288px)
int ledSpacing = 1;         // Spacing between LEDs (default: 1, reduce to 0 for tighter spacing)

// Color definitions (RGB565 format)
#define LED_COLOR         0xF800 // Red color for LEDs
#define BG_COLOR          0x0000 // Black background
#define LED_OFF_COLOR     0x2000 // Dim red for "off" LEDs

// Calculate display dimensions for centering (now calculated dynamically based on ledSize)
// Plus 4-pixel gap between the two matrix rows (authentic spacing)
int getDisplayWidth() { return ledSize * TOTAL_WIDTH; }
int getDisplayHeight() { return ledSize * TOTAL_HEIGHT + 4; }

// ======================== DISPLAY STYLE CONFIGURATION ========================
#define DEFAULT_DISPLAY_STYLE 1  // Start with realistic style

// Color presets (RGB565 format)
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
#define COLOR_BLACK       0x0000

// ======================== TIMING CONFIGURATION ========================
#define SENSOR_UPDATE_INTERVAL       60000  // Update sensor every 60s
#define NTP_SYNC_INTERVAL            3600000 // Sync NTP every hour
#define STATUS_PRINT_INTERVAL        60000  // Print status every 60s

// ======================== DEBUG CONFIGURATION ========================
#define DEBUG_ENABLED 1

// ======================== DISPLAY OPTIMIZATION ========================
#define BRIGHTNESS_BOOST 1  // Set to 1 for maximum brightness
#define FAST_REFRESH 1      // Set to 1 to only redraw changed pixels

#if DEBUG_ENABLED
  #define DEBUG(x) x
  #define DEBUG_SETTINGS(x) if(settingsChanged) { x; settingsChanged = false; }
#else
  #define DEBUG(x)
  #define DEBUG_SETTINGS(x)
#endif

// ======================== DISPLAY OBJECT ========================
TFT_eSPI tft = TFT_eSPI();  // TFT_eSPI uses configuration from User_Setup.h

// ======================== DISPLAY BUFFER ========================
// Virtual screen buffer matching original LED matrix structure
byte scr[LINE_WIDTH * DISPLAY_ROWS]; // 32 columns × 2 rows = 64 bytes

// ======================== GLOBAL OBJECTS ========================
WebServer server(80);
WiFiManager wifiManager;

// Sensor objects (only one will be initialized based on configuration)
#ifdef USE_BME280
  Adafruit_BME280 bme280;
#endif
#ifdef USE_SHT3X
  Adafruit_SHT31 sht3x = Adafruit_SHT31();
#endif
#ifdef USE_HTU21D
  Adafruit_HTU21DF htu21d = Adafruit_HTU21DF();
#endif

// ======================== FONT INCLUDES ========================
#include "fonts.h"
#include "timezones.h"

// ======================== TIME VARIABLES ========================
int hours = 0, minutes = 0, seconds = 0;
int hours24 = 0;  // 24-hour format
int day = 1, month = 1, year = 2025;
int lastSecond = -1;
bool use24HourFormat = false;  // Default to 12-hour format
bool showLeadingZero = false;  // Default: no leading zero for hours < 10

// Date format: 0=DD/MM/YY, 1=MM/DD/YY, 2=YYYY-MM-DD, 3=DD.MM.YYYY, 4=MM.DD.YYYY
int dateFormat = 0;

// ======================== SENSOR VARIABLES ========================
bool sensorAvailable = false;
int temperature = 0;
int humidity = 0;
int pressure = 0;
bool useFahrenheit = false;
const char* sensorType = "NONE";  // Will be set based on detected sensor

// ======================== TIMING VARIABLES ========================
unsigned long lastSensorUpdate = 0;
unsigned long lastNTPSync = 0;
unsigned long lastStatusPrint = 0;

// ======================== DISPLAY STYLE VARIABLES ========================
int displayStyle = DEFAULT_DISPLAY_STYLE;  // 0=Default, 1=Realistic
uint16_t ledOnColor = COLOR_RED;           // Color for lit LEDs
uint16_t ledSurroundColor = COLOR_RED;     // Surround color (default: red)
uint16_t ledOffColor = 0x2000;             // Color for unlit LEDs (dim)
bool surroundMatchesLED = false;           // Track if surround should match LED color
bool forceFullRedraw = false;              // Flag to force immediate complete redraw
bool settingsChanged = false;              // Flag to trigger detailed debug output
uint8_t displayRotation = 1;               // Display rotation: 1=normal, 3=180° flipped

// ======================== DISPLAY MODES ========================
int currentMode = 0; // 0=Time+Temp, 1=Time Large, 2=Time+Date
unsigned long lastModeSwitch = 0;
#define MODE_SWITCH_INTERVAL 5000  // Default interval (not used directly)
int modeSwitchInterval = 5;  // Mode switch interval in seconds (default: 5, range: 1-60)

// ======================== TIMEZONE ========================
int currentTimezone = 0;

// ======================== RGB LED FUNCTIONS ========================
void setRGBLed(bool red, bool green, bool blue) {
  // CYD RGB LEDs are active LOW
  digitalWrite(LED_R_PIN, red ? LOW : HIGH);
  digitalWrite(LED_G_PIN, green ? LOW : HIGH);
  digitalWrite(LED_B_PIN, blue ? LOW : HIGH);
}

void flashRGBLed(int r, int g, int b, int delayMs = 200) {
  setRGBLed(r, g, b);
  delay(delayMs);
  setRGBLed(false, false, false);
}

// ======================== TFT DISPLAY FUNCTIONS ========================

void initTFT() {
  DEBUG(Serial.println("Initializing TFT Display..."));

  // Setup backlight pin
  pinMode(TFT_BL_PIN, OUTPUT);
  digitalWrite(TFT_BL_PIN, HIGH);  // Turn backlight ON
  DEBUG(Serial.println("Backlight enabled"));

  // Add small delay before TFT initialization
  delay(100);

  // TFT_eSPI initialization
  tft.init();
  tft.setRotation(displayRotation);  // Rotation 1 = landscape mode (320x240), 3 = 180° flipped
  DEBUG(Serial.printf("TFT_eSPI initialized, rotation set to %d\n", displayRotation));

  // Small delay after initialization
  delay(100);

  // Check actual dimensions
  DEBUG(Serial.printf("TFT reports dimensions: %d x %d\n", tft.width(), tft.height()));

  tft.fillScreen(BG_COLOR);

  // Calculate display dimensions
  int displayWidth = tft.width();
  int displayHeight = tft.height();
  int matrixWidth = getDisplayWidth();
  int matrixHeight = getDisplayHeight();
  int offsetX = ((displayWidth - matrixWidth) / 2) > 0 ? ((displayWidth - matrixWidth) / 2) : 0;
  int offsetY = ((displayHeight - matrixHeight) / 2) > 0 ? ((displayHeight - matrixHeight) / 2) : 0;

  DEBUG(Serial.printf("TFT Display initialized: %dx%d\n", displayWidth, displayHeight));
  DEBUG(Serial.printf("LED Matrix area: %dx%d at offset (%d,%d)\n",
        matrixWidth, matrixHeight, offsetX, offsetY));
  
  if (displayWidth <= 0 || displayHeight <= 0) {
    DEBUG(Serial.println("ERROR: Invalid TFT dimensions!"));
    DEBUG(Serial.println("Check TFT configuration in User_Setup.h"));
  }
}

void clearScreen() {
  for (int i = 0; i < LINE_WIDTH * DISPLAY_ROWS; i++) {
    scr[i] = 0;
  }
}

// Dim an RGB565 color while preserving hue
uint16_t dimRGB565(uint16_t color, int factor) {
  int r = (color >> 11) & 0x1F;
  int g = (color >> 5) & 0x3F;
  int b = color & 0x1F;
  
  r = r / (factor + 1);
  g = g / (factor + 1);
  b = b / (factor + 1);
  
  return (r << 11) | (g << 5) | b;
}

void forceCompleteRefresh() {
  tft.fillScreen(BG_COLOR);
  clearScreen();
}

void drawLEDPixel(int x, int y, bool lit) {
  // Bounds checking
  if (x < 0 || x >= TOTAL_WIDTH || y < 0 || y >= TOTAL_HEIGHT) {
    return;
  }
  
  // Calculate screen position with centering offset
  int displayWidth = getDisplayWidth();
  int displayHeight = getDisplayHeight();
  int offsetX = ((tft.width() - displayWidth) / 2) > 0 ? ((tft.width() - displayWidth) / 2) : 0;
  int offsetY = ((tft.height() - displayHeight) / 2) > 0 ? ((tft.height() - displayHeight) / 2) : 0;

  // Add extra gap between matrix rows (after row 7, before row 8)
  int matrixGap = (y >= 8) ? 4 : 0;  // 4-pixel gap between matrix rows

  int screenX = offsetX + x * ledSize;
  int screenY = offsetY + y * ledSize + matrixGap;
  
  if (displayStyle == 0) {
    // ========== DEFAULT STYLE: Solid square blocks ==========
    uint16_t color = lit ? ledOnColor : BG_COLOR;
    tft.fillRect(screenX, screenY, ledSize, ledSize, color);
  }
  else {
    // ========== REALISTIC STYLE: Circular LED with surround ==========
    int center = ledSize / 2;
    int ledRadius = (ledSize - 2) / 2;  // Leave 1px border

    if (!lit) {
      // OFF LED: Show dark circle
      tft.fillRect(screenX, screenY, ledSize, ledSize, BG_COLOR);

      uint16_t offHousing = dimRGB565(ledSurroundColor, 7);
      uint16_t offLED = 0x1800;  // Very dark red

      for (int py = 1; py < ledSize - 1; py++) {
        for (int px = 1; px < ledSize - 1; px++) {
          int dx = (px * 2 - ledSize + 1);
          int dy = (py * 2 - ledSize + 1);
          int distSq = dx * dx + dy * dy;
          int threshInner = (ledSize - 4) * (ledSize - 4);
          int threshOuter = (ledSize - 2) * (ledSize - 2);

          if (distSq <= threshInner) {
            tft.drawPixel(screenX + px, screenY + py, offLED);
          }
          else if (distSq <= threshOuter) {
            tft.drawPixel(screenX + px, screenY + py, offHousing);
          }
        }
      }
    }
    else {
      // LIT LED: Draw bright circular LED with surround
      tft.fillRect(screenX, screenY, ledSize, ledSize, ledSurroundColor);

      for (int py = 0; py < ledSize; py++) {
        for (int px = 0; px < ledSize; px++) {
          int dx = (px * 2 - ledSize + 1);
          int dy = (py * 2 - ledSize + 1);
          int distSq = dx * dx + dy * dy;

          uint16_t pixelColor;
          int threshCore = (ledSize - 6) * (ledSize - 6);
          int threshBody = (ledSize - 2) * (ledSize - 2);
          int threshSurround = ledSize * ledSize;

          if (distSq <= threshCore) {
            pixelColor = ledOnColor;
          }
          else if (distSq <= threshBody) {
            pixelColor = ledOnColor;
          }
          else if (distSq <= threshSurround) {
            pixelColor = ledSurroundColor;
          }
          else {
            pixelColor = BG_COLOR;
          }

          tft.drawPixel(screenX + px, screenY + py, pixelColor);
        }
      }
    }
  }
}

// Format date according to selected format
void formatDate(char* buf, size_t bufSize, int d, int m, int y) {
  switch(dateFormat) {
    case 0: // DD/MM/YY
      snprintf(buf, bufSize, "%02d/%02d/%02d", d, m, y % 100);
      break;
    case 1: // MM/DD/YY
      snprintf(buf, bufSize, "%02d/%02d/%02d", m, d, y % 100);
      break;
    case 2: // YYYY-MM-DD (ISO 8601)
      snprintf(buf, bufSize, "%04d-%02d-%02d", y, m, d);
      break;
    case 3: // DD.MM.YYYY
      snprintf(buf, bufSize, "%02d.%02d.%04d", d, m, y);
      break;
    case 4: // MM.DD.YYYY
      snprintf(buf, bufSize, "%02d.%02d.%04d", m, d, y);
      break;
    default: // Fallback to DD/MM/YY
      snprintf(buf, bufSize, "%02d/%02d/%02d", d, m, y % 100);
  }
}

void refreshAll() {
  #if FAST_REFRESH
    static byte lastScr[LINE_WIDTH * DISPLAY_ROWS] = {0};
    static bool firstRun = true;
    
    if (forceFullRedraw) {
      for (int i = 0; i < LINE_WIDTH * DISPLAY_ROWS; i++) {
        lastScr[i] = 0xFF;
      }
      forceFullRedraw = false;
      firstRun = true;
      DEBUG(Serial.println("FAST_REFRESH cache cleared - forcing full redraw"));
    }
  #endif
  
  for (int row = 0; row < DISPLAY_ROWS; row++) {
    for (int displayX = 0; displayX < LINE_WIDTH; displayX++) {
      int bufferIndex = displayX + row * LINE_WIDTH;
      
      if (bufferIndex >= 0 && bufferIndex < LINE_WIDTH * DISPLAY_ROWS) {
        byte pixelByte = scr[bufferIndex];
        
        #if FAST_REFRESH
          if (firstRun || pixelByte != lastScr[bufferIndex]) {
            lastScr[bufferIndex] = pixelByte;
        #endif
            
            for (int bitPos = 0; bitPos < 8; bitPos++) {
              int displayY = row * 8 + bitPos;
              bool lit = (pixelByte & (1 << bitPos)) != 0;
              
              drawLEDPixel(displayX, displayY, lit);
            }
            
        #if FAST_REFRESH
          }
        #endif
      }
    }
  }
  
  #if FAST_REFRESH
    firstRun = false;
  #endif
}

void invert() {
  for (int i = 0; i < LINE_WIDTH * DISPLAY_ROWS; i++) {
    scr[i] = ~scr[i];
  }
}

void scrollLeft() {
  for (int i = 0; i < LINE_WIDTH * DISPLAY_ROWS - 1; i++) {
    scr[i] = scr[i + 1];
  }
  scr[LINE_WIDTH * DISPLAY_ROWS - 1] = 0;
}

// ======================== FONT HELPER FUNCTIONS ========================

int charWidth(char c, const uint8_t* font) {
  int firstChar = pgm_read_byte(font + 2);
  int lastChar = pgm_read_byte(font + 3);
  
  if (c < firstChar || c > lastChar) return 0;
  
  int charIndex = c - firstChar;
  int offset = 4;
  for (int i = 0; i < charIndex; i++) {
    int w = pgm_read_byte(font + offset);
    offset += w * pgm_read_byte(font + 1) + 1;
  }
  
  return pgm_read_byte(font + offset);
}

int drawCharWithY(int x, int yPos, char c, const uint8_t* font);

int drawChar(int x, char c, const uint8_t* font) {
  return drawCharWithY(x, 0, c, font);
}

int drawCharWithY(int x, int yPos, char c, const uint8_t* font) {
  int fwd = pgm_read_byte(font);
  int fht = pgm_read_byte(font + 1);
  int offs = pgm_read_byte(font + 2);
  int last = pgm_read_byte(font + 3);
  
  if (c < offs || c > last) return 0;
  
  c -= offs;
  int fht8 = (fht + 7) / 8;
  font += 4 + c * (fht8 * fwd + 1);
  
  int j, i, w = pgm_read_byte(font);
  
  for (j = 0; j < fht8; j++) {
    for (i = 0; i < w; i++) {
      if (x + i >= 0 && x + i < LINE_WIDTH) {
        int bufferIndex = x + LINE_WIDTH * (j + yPos) + i;
        if (bufferIndex >= 0 && bufferIndex < LINE_WIDTH * DISPLAY_ROWS) {
          scr[bufferIndex] = pgm_read_byte(font + 1 + fht8 * i + j);
        }
      }
    }
    if (x + i < LINE_WIDTH && x + i >= 0) {
      int bufferIndex = x + LINE_WIDTH * (j + yPos) + i;
      if (bufferIndex >= 0 && bufferIndex < LINE_WIDTH * DISPLAY_ROWS) {
        scr[bufferIndex] = 0;
      }
    }
  }
  
  return w;
}

int stringWidth(const char* str, const uint8_t* font) {
  int width = 0;
  while (*str) {
    width += charWidth(*str++, font) + 1;
  }
  return width - 1;
}

void showMessage(const char* msg) {
  if (msg == NULL || strlen(msg) == 0) return;

  clearScreen();
  delay(10);

  int width = stringWidth(msg, font3x7);
  int x = (TOTAL_WIDTH - width) / 2;

  if (x < 0) x = 0;
  if (x >= TOTAL_WIDTH) x = TOTAL_WIDTH - 1;

  while (*msg) {
    x += drawChar(x, *msg++, font3x7) + 1;
  }

  delay(10);
  refreshAll();
}

void showIPAddress(const char* ip) {
  if (ip == NULL || strlen(ip) == 0) return;

  clearScreen();
  delay(10);

  // Split IP address at the second dot
  // Example: "192.168.1.123" -> "IP: 192.168." (top) and "1.123" (bottom)
  String ipStr = String(ip);
  int secondDot = ipStr.indexOf('.', ipStr.indexOf('.') + 1);

  if (secondDot > 0) {
    String topLine = "IP:" + ipStr.substring(0, secondDot) + ".";
    String bottomLine = ipStr.substring(secondDot + 1);

    // Display top line (IP: first two octets with trailing dot)
    int topX = 0;  // Left-aligned
    const char* topMsg = topLine.c_str();
    while (*topMsg) {
      topX += drawCharWithY(topX, 0, *topMsg++, font3x7) + 1;
    }

    // Display bottom line (last two octets)
    int bottomX = 0;  // Left-aligned
    const char* bottomMsg = bottomLine.c_str();
    while (*bottomMsg) {
      bottomX += drawCharWithY(bottomX, 1, *bottomMsg++, font3x7) + 1;
    }
  } else {
    // Fallback to single line if split fails
    String fallback = "IP:" + String(ip);
    int x = 0;  // Left-aligned

    const char* msg = fallback.c_str();
    while (*msg) {
      x += drawChar(x, *msg++, font3x7) + 1;
    }
  }

  delay(10);
  refreshAll();
}

// ======================== DISPLAY FUNCTIONS ========================

void displayTimeAndTemp() {
  clearScreen();
  
  char buf[32];
  bool showDots = (seconds % 2) == 0;
  
  int x = 0;
  int displayHours = use24HourFormat ? hours24 : hours;

  // Hours (using font3x7 to match temperature display size)
  if (showLeadingZero) {
    sprintf(buf, "%02d", displayHours);  // With leading zero
  } else {
    sprintf(buf, "%d", displayHours);    // No leading zero
  }
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, font3x7);
    if (*(p+1)) x++;
  }

  // Colon - Mode 0: flashing colon (one LED space before)
  x++;  // Space before colon
  if (showDots) {
    x += drawCharWithY(x, 0, ':', font3x7);
    x++;  // Space after colon
  } else {
    x += 2;  // When colon hidden, maintain spacing
  }

  // Minutes (using font3x7 to match temperature display size)
  sprintf(buf, "%02d", minutes);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, font3x7);
    if (*(p+1)) x++;
  }
  
  // AM/PM indicator (only in 12-hour mode)
  // In 12-hour mode, display AM or PM
  // In 24-hour mode, nothing is displayed after minutes

  if (!use24HourFormat) {
    // Display AM or PM in 12-hour mode
    const char* ampm = (hours24 >= 12) ? "PM" : "AM";

    x++;  // Space before AM/PM
    for (const char* p = ampm; *p; p++) {
      if (x < LINE_WIDTH) {
        x += drawCharWithY(x, 0, *p, font3x7);
        if (*(p+1) && x < LINE_WIDTH) x++;
      }
    }
  }
  // Note: Seconds are not displayed in Mode 0
  
  // Bottom row: Temperature and Humidity
  x = 0;
  if (sensorAvailable) {
    int displayTemp = useFahrenheit ? (temperature * 9 / 5 + 32) : temperature;
    char tempUnit = useFahrenheit ? 'F' : 'C';
    sprintf(buf, "T%d%c H%d%%", displayTemp, tempUnit, humidity);
  } else {
    sprintf(buf, "NO SENSOR");
  }
  
  for (const char* p = buf; *p; p++) {
    if (x < LINE_WIDTH - 3) {
      x += drawCharWithY(x, 1, *p, font3x7);
      if (*(p+1) && x < LINE_WIDTH) x++;
    }
  }
}

void displayTimeLarge() {
  clearScreen();
  
  char buf[32];
  bool showDots = (seconds % 2) == 0;
  
  int displayHours = use24HourFormat ? hours24 : hours;
  int x = (displayHours > 9 || showLeadingZero) ? 0 : 3;

  // Draw hours
  if (showLeadingZero) {
    sprintf(buf, "%02d", displayHours);  // With leading zero
  } else {
    sprintf(buf, "%d", displayHours);    // No leading zero
  }
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x16rn);
    if (*(p+1)) x++;
  }

  // Draw colon - Mode 1: one LED space before and after the colon
  x++;  // Space before colon
  if (showDots) {
    x += drawCharWithY(x, 0, ':', digits5x16rn);
    x++;  // Space after colon
  } else {
    x += 2;  // When colon is not shown, still maintain spacing
  }

  // Draw minutes
  sprintf(buf, "%02d", minutes);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x16rn);
    if (*(p+1)) x++;
  }

  // Add seconds in small font
  // Note: May be truncated in 24-hour mode with hours >= 10 at larger LED sizes
  // Users can adjust LED Size or Spacing via web interface to fit all elements
  x++;
  sprintf(buf, "%02d", seconds);
  for (const char* p = buf; *p; p++) {
    if (x < LINE_WIDTH) {
      x += drawCharWithY(x, 0, *p, font3x7);
      if (*(p+1) && x < LINE_WIDTH) x++;
    }
  }
}

void displayTimeAndDate() {
  clearScreen();
  
  char buf[32];
  bool showDots = (seconds % 2) == 0;
  
  int displayHours = use24HourFormat ? hours24 : hours;

  // Top row: Time
  int x = 0;
  if (showLeadingZero) {
    sprintf(buf, "%02d", displayHours);  // With leading zero
  } else {
    sprintf(buf, "%d", displayHours);    // No leading zero
  }
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x8rn);
    if (*(p+1)) x++;
  }

  // Colon - Mode 2: one LED space before the colon
  x++;  // Space before colon
  if (showDots) {
    x += drawCharWithY(x, 0, ':', digits5x8rn);
    x += 1;
  } else {
    x += 2;
  }

  sprintf(buf, "%02d", minutes);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x8rn);
    if (*(p+1)) x++;
  }

  // Add seconds
  // Note: May be truncated in 24-hour mode with hours >= 10 at larger LED sizes
  // Users can adjust LED Size or Spacing via web interface to fit all elements
  x++;
  sprintf(buf, "%02d", seconds);
  for (const char* p = buf; *p; p++) {
    if (x < LINE_WIDTH) {
      x += drawCharWithY(x, 0, *p, digits3x5);
      if (*(p+1) && x < LINE_WIDTH) x++;
    }
  }
  
  // Bottom row: Date
  x = 2;
  formatDate(buf, sizeof(buf), day, month, year);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 1, *p, font3x7) + 1;
  }
}

// ======================== SENSOR FUNCTIONS ========================

bool testSensor() {
  Wire.begin(SDA_PIN, SCL_PIN);

#ifdef USE_BME280
  // Test BME280 sensor
  if (!bme280.begin(0x76, &Wire)) {
    DEBUG(Serial.println("BME280 sensor not found at 0x76"));
    if (!bme280.begin(0x77, &Wire)) {
      DEBUG(Serial.println("BME280 sensor not found at 0x77 either"));
      return false;
    }
  }

  bme280.setSampling(Adafruit_BME280::MODE_FORCED,
                     Adafruit_BME280::SAMPLING_X1,
                     Adafruit_BME280::SAMPLING_X1,
                     Adafruit_BME280::SAMPLING_X1,
                     Adafruit_BME280::FILTER_OFF);

  float temp = bme280.readTemperature();
  float hum = bme280.readHumidity();

  if (isnan(temp) || isnan(hum) || temp < -50 || temp > 100 || hum < 0 || hum > 100) {
    DEBUG(Serial.println("BME280 readings invalid"));
    return false;
  }

  DEBUG(Serial.printf("BME280 OK: %.1f°C, %.1f%%\n", temp, hum));
  sensorType = "BME280";
  return true;

#elif defined(USE_SHT3X)
  // Test SHT3X sensor
  if (!sht3x.begin(0x44)) {  // Default I2C address for SHT3X
    DEBUG(Serial.println("SHT3X sensor not found at 0x44"));
    if (!sht3x.begin(0x45)) {  // Alternative I2C address
      DEBUG(Serial.println("SHT3X sensor not found at 0x45 either"));
      return false;
    }
  }

  // Read initial values to verify sensor is working
  float temp = sht3x.readTemperature();
  float hum = sht3x.readHumidity();

  if (isnan(temp) || isnan(hum) || temp < -50 || temp > 100 || hum < 0 || hum > 100) {
    DEBUG(Serial.println("SHT3X readings invalid"));
    return false;
  }

  DEBUG(Serial.printf("SHT3X OK: %.1f°C, %.1f%%\n", temp, hum));
  sensorType = "SHT3X";
  return true;

#elif defined(USE_HTU21D)
  // Test HTU21D sensor
  if (!htu21d.begin()) {  // HTU21D uses fixed I2C address 0x40
    DEBUG(Serial.println("HTU21D sensor not found at 0x40"));
    return false;
  }

  // Read initial values to verify sensor is working
  float temp = htu21d.readTemperature();
  float hum = htu21d.readHumidity();

  if (isnan(temp) || isnan(hum) || temp < -50 || temp > 100 || hum < 0 || hum > 100) {
    DEBUG(Serial.println("HTU21D readings invalid"));
    return false;
  }

  DEBUG(Serial.printf("HTU21D OK: %.1f°C, %.1f%%\n", temp, hum));
  sensorType = "HTU21D";
  return true;

#else
  DEBUG(Serial.println("No sensor type defined in configuration"));
  return false;
#endif
}

void updateSensorData() {
  if (!sensorAvailable) return;

  float temp = NAN;
  float hum = NAN;
  float pres = NAN;

#ifdef USE_BME280
  // Read BME280 sensor
  bme280.takeForcedMeasurement();
  temp = bme280.readTemperature();
  hum = bme280.readHumidity();
  pres = bme280.readPressure() / 100.0F;

#elif defined(USE_SHT3X)
  // Read SHT3X sensor
  temp = sht3x.readTemperature();
  hum = sht3x.readHumidity();
  // SHT3X doesn't have a pressure sensor, so pressure remains NAN

#elif defined(USE_HTU21D)
  // Read HTU21D sensor
  temp = htu21d.readTemperature();
  hum = htu21d.readHumidity();
  // HTU21D doesn't have a pressure sensor, so pressure remains NAN
#endif

  // Update temperature if valid
  if (!isnan(temp) && temp >= -50 && temp <= 100) {
    temperature = (int)round(temp);
  }

  // Update humidity if valid
  if (!isnan(hum) && hum >= 0 && hum <= 100) {
    humidity = (int)round(hum);
  }

  // Update pressure if valid (only for BME280)
  if (!isnan(pres) && pres >= 800 && pres <= 1200) {
    pressure = (int)round(pres);
  }
}

// ======================== NTP SYNC FUNCTION ========================

void syncNTP() {
  DEBUG(Serial.println("Syncing time with NTP..."));
  
  // ESP32 uses configTzTime instead of configTime with TZ string
  configTzTime(timezones[currentTimezone].tzString, "pool.ntp.org", "time.nist.gov");
  
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 24 * 3600 && attempts < 20) {
    delay(500);
    now = time(nullptr);
    attempts++;
  }
  
  if (now > 24 * 3600) {
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    hours = timeinfo.tm_hour % 12;
    if (hours == 0) hours = 12;
    hours24 = timeinfo.tm_hour;
    minutes = timeinfo.tm_min;
    seconds = timeinfo.tm_sec;
    day = timeinfo.tm_mday;
    month = timeinfo.tm_mon + 1;
    year = timeinfo.tm_year + 1900;
    
    DEBUG(Serial.printf("Time synced: %02d:%02d:%02d %02d/%02d/%d (TZ: %s)\n",
                        hours24, minutes, seconds, day, month, year,
                        timezones[currentTimezone].name));
    
    // Flash green LED on successful sync
    flashRGBLed(0, 1, 0);
  } else {
    DEBUG(Serial.println("NTP sync failed"));
    // Flash red LED on failed sync
    flashRGBLed(1, 0, 0);
  }
}

// ======================== TIME UPDATE FUNCTION ========================

void updateTime() {
  time_t now = time(nullptr);
  if (now < 24 * 3600) return;
  
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  
  hours = timeinfo.tm_hour % 12;
  if (hours == 0) hours = 12;
  hours24 = timeinfo.tm_hour;
  minutes = timeinfo.tm_min;
  seconds = timeinfo.tm_sec;
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon + 1;
  year = timeinfo.tm_year + 1900;
  
  if (seconds != lastSecond) {
    lastSecond = seconds;

    // Show what's being displayed in current mode
    if (currentMode == 0) {
      // Mode 0: Time + Temp
      if (!use24HourFormat) {
        int displayHours = hours24 % 12;
        if (displayHours == 0) displayHours = 12;
        const char* ampm = (hours24 >= 12) ? "PM" : "AM";
        DEBUG(Serial.printf("Mode 0: %d:%02d %s | Temp: %d°%c Hum: %d%%\n",
          displayHours, minutes, ampm,
          useFahrenheit ? (temperature * 9 / 5 + 32) : temperature,
          useFahrenheit ? 'F' : 'C',
          humidity));
      } else {
        DEBUG(Serial.printf("Mode 0: %02d:%02d | Temp: %d°%c Hum: %d%%\n",
          hours24, minutes,
          useFahrenheit ? (temperature * 9 / 5 + 32) : temperature,
          useFahrenheit ? 'F' : 'C',
          humidity));
      }
    } else if (currentMode == 1) {
      // Mode 1: Large Time
      int displayHours = use24HourFormat ? hours24 : hours;
      DEBUG(Serial.printf("Mode 1: %02d:%02d:%02d (Large)\n", displayHours, minutes, seconds));
    } else if (currentMode == 2) {
      // Mode 2: Time + Date
      int displayHours = use24HourFormat ? hours24 : hours;
      DEBUG(Serial.printf("Mode 2: %02d:%02d:%02d | %02d/%02d/%04d\n",
        displayHours, minutes, seconds, day, month, year));
    }

    switch (currentMode) {
      case 0: displayTimeAndTemp(); break;
      case 1: displayTimeLarge(); break;
      case 2: displayTimeAndDate(); break;
    }
    refreshAll();
  }

  // Auto-switch modes (using user-configurable interval)
  if (millis() - lastModeSwitch > (modeSwitchInterval * 1000)) {
    currentMode = (currentMode + 1) % 3;
    lastModeSwitch = millis();
  }
}

// ======================== WEB SERVER FUNCTIONS ========================

void setupWebServer() {
  // Root page handler
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>CYD LED Clock</title>";
    html += "<style>";
    html += "*{box-sizing:border-box;}";
    html += "body{font-family:'Segoe UI',Arial,sans-serif;margin:0;padding:10px;background:#1a1a1a;color:#fff;max-width:1200px;margin:0 auto;}";
    html += ".header{text-align:center;margin-bottom:12px;}";
    html += "h1{color:#fff;font-size:clamp(20px,5vw,26px);font-weight:600;margin:0 0 15px 0;}";
    html += ".time-display{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:clamp(15px,4vw,25px);border-radius:12px;box-shadow:0 4px 16px rgba(0,0,0,0.3);margin-bottom:12px;}";
    html += ".time-display h2{color:#aaa;font-size:clamp(14px,4vw,18px);font-weight:400;margin:0 0 10px 0;text-align:left;}";
    html += ".clock{font-size:clamp(40px,12vw,90px);font-weight:700;text-align:center;margin:10px 0;font-family:'Courier New',monospace;color:#7CFC00;text-shadow:0 0 20px rgba(124,252,0,0.5);line-height:1.1;}";
    html += ".date{font-size:clamp(20px,6vw,38px);font-weight:600;text-align:center;margin:10px 0;font-family:'Courier New',monospace;color:#4A90E2;text-shadow:0 0 15px rgba(74,144,226,0.5);line-height:1.2;}";
    html += ".environment{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:clamp(15px,3vw,25px);border-radius:12px;box-shadow:0 4px 16px rgba(0,0,0,0.3);margin-bottom:12px;}";
    html += ".environment p{margin:6px 0;}";
    html += ".env-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:clamp(12px,3vw,20px);text-align:center;}";
    html += ".env-item{padding:clamp(12px,3vw,16px);background:rgba(255,255,255,0.05);border-radius:8px;transition:transform 0.2s;}";
    html += ".env-item:hover{transform:translateY(-3px);background:rgba(255,255,255,0.08);}";
    html += ".env-icon{font-size:clamp(32px,8vw,48px);margin-bottom:6px;display:block;}";
    html += ".env-value{font-size:clamp(20px,5vw,30px);font-weight:700;margin:6px 0;font-family:'Courier New',monospace;line-height:1.2;}";
    html += ".env-label{font-size:clamp(11px,3vw,14px);color:#aaa;text-transform:uppercase;letter-spacing:0.5px;}";
    html += ".card{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:clamp(12px,3vw,16px);margin:8px 0;border-radius:8px;box-shadow:0 3px 12px rgba(0,0,0,0.3);}";
    html += "h2{color:#aaa;border-bottom:2px solid #4CAF50;padding-bottom:4px;font-size:clamp(15px,4vw,17px);font-weight:500;margin:0 0 10px 0;}";
    html += "button{background:#4CAF50;color:white;border:none;padding:8px 12px;cursor:pointer;border-radius:5px;margin:4px 4px 4px 0;font-size:clamp(12px,3vw,13px);white-space:nowrap;}";
    html += "button:hover{background:#45a049;}";
    html += "select{padding:6px;font-size:clamp(12px,3vw,13px);background:#1e1e1e;color:#fff;border:1px solid #444;border-radius:5px;width:100%;max-width:280px;}";
    html += "p{color:#ccc;font-size:clamp(12px,3vw,14px);line-height:1.5;margin:6px 0;}";
    html += ".status-pill{display:inline-block;padding:4px 10px;border-radius:999px;font-size:12px;font-weight:700;letter-spacing:0.3px;border:1px solid #2e7d32;background:#1f3b23;color:#9CFF9C;}";
    html += ".status-subtext{display:block;color:#aaa;font-size:12px;margin-top:4px;}";
    html += ".note{background:rgba(255,255,255,0.04);border:1px dashed #555;padding:8px;border-radius:6px;color:#ccc;font-size:12px;margin-top:8px;line-height:1.5;}";
    html += "@media(max-width:768px){";
    html += ".env-grid{grid-template-columns:1fr;}";
    html += ".clock{font-size:clamp(40px,12vw,80px);}";
    html += ".date{font-size:clamp(20px,6vw,36px);}";
    html += "body{padding:8px;}";
    html += ".time-display,.environment,.card{padding:12px;}";
    html += "}";
    // TFT Display Mirror styles
    html += ".tft-mirror{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:clamp(12px,3vw,20px);border-radius:12px;box-shadow:0 4px 16px rgba(0,0,0,0.3);margin-bottom:12px;text-align:center;}";
    html += ".tft-mirror h2{color:#aaa;border-bottom:2px solid #E91E63;padding-bottom:4px;font-size:clamp(15px,4vw,17px);font-weight:500;margin:0 0 10px 0;text-align:left;}";
    html += ".canvas-container{display:flex;justify-content:center;align-items:center;padding:12px;background:#000;border-radius:8px;margin-top:10px;}";
    html += "#tftCanvas{image-rendering:pixelated;image-rendering:crisp-edges;border:2px solid #444;border-radius:4px;box-shadow:0 0 8px rgba(68,68,68,0.5);}";
    html += ".tft-label{color:#888;font-size:11px;margin-top:8px;}";
    html += ".footer{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:16px;margin:12px 0 0 0;border-radius:8px;box-shadow:0 3px 12px rgba(0,0,0,0.3);text-align:center;}";
    html += ".footer-content{display:flex;align-items:center;justify-content:center;gap:8px;flex-wrap:wrap;margin-bottom:12px;}";
    html += ".footer-link{color:#4CAF50;text-decoration:none;font-size:clamp(14px,3.5vw,16px);font-weight:500;transition:color 0.3s;}";
    html += ".footer-link:hover{color:#66BB6A;}";
    html += ".footer-separator{color:#666;font-size:clamp(14px,3.5vw,16px);}";
    html += ".footer-heart{color:#E91E63;font-size:clamp(14px,3.5vw,16px);}";
    html += ".footer-credit{color:#888;font-size:clamp(11px,3vw,13px);margin-top:8px;line-height:1.6;}";
    html += ".footer-credit a{color:#4A90E2;text-decoration:none;}";
    html += ".footer-credit a:hover{color:#6BA9E8;text-decoration:underline;}";
    html += "</style>";
    html += "<script>";
    html += "function formatDate(day,month,year,fmt){";
    html += "var d=(day<10?'0':'')+day,m=(month<10?'0':'')+month,y2=(''+year).slice(-2),y4=year;";
    html += "if(fmt===0)return d+'/'+m+'/'+y2;";
    html += "if(fmt===1)return m+'/'+d+'/'+y2;";
    html += "if(fmt===2)return y4+'-'+m+'-'+d;";
    html += "if(fmt===3)return d+'.'+m+'.'+y4;";
    html += "if(fmt===4)return m+'.'+d+'.'+y4;";
    html += "return d+'/'+m+'/'+y2;";
    html += "}";
    html += "function updateTime(){";
    html += "fetch('/api/time')";
    html += ".then(function(r){return r.json();})";
    html += ".then(function(d){";
    html += "var clock=document.getElementById('clock');";
    html += "var date=document.getElementById('date');";
    html += "var h=d.hours;";
    html += "var ampm='';";
    html += "if(!d.use24hour){";
    html += "ampm=(h>=12)?' PM':' AM';";
    html += "h=(h%12)||12;";
    html += "}";
    html += "if(clock){clock.textContent=(d.use24hour&&h<10?'0':'')+h+':'+(d.minutes<10?'0':'')+d.minutes+':'+(d.seconds<10?'0':'')+d.seconds+ampm;}";
    html += "if(date){date.textContent=formatDate(d.day,d.month,d.year,d.dateFormat);}";
    html += "})";
    html += ".catch(function(e){console.log('Update failed:',e);});";
    html += "}";
    html += "setInterval(updateTime,1000);";
    html += "setTimeout(updateTime,100);";
    // TFT Display Mirror - Canvas rendering functions
    html += "var tftCanvas,tftCtx,ledSize=9,gapSize=4;";
    html += "function rgb565ToHex(c){var r=((c>>11)&0x1F)*8,g=((c>>5)&0x3F)*4,b=(c&0x1F)*8;return'rgb('+r+','+g+','+b+')';}";
    html += "function dimColor(r,g,b,f){return'rgb('+Math.floor(r/f)+','+Math.floor(g/f)+','+Math.floor(b/f)+')';}";
    html += "function initCanvas(){";
    html += "tftCanvas=document.getElementById('tftCanvas');";
    html += "if(!tftCanvas)return;";
    html += "tftCtx=tftCanvas.getContext('2d');";
    html += "tftCanvas.width=32*ledSize;";
    html += "tftCanvas.height=16*ledSize+gapSize;";
    html += "tftCtx.fillStyle='#000';tftCtx.fillRect(0,0,tftCanvas.width,tftCanvas.height);";
    html += "}";
    html += "function drawLED(x,y,lit,style,ledColor,surroundColor){";
    html += "var gap=(y>=8)?gapSize:0;";
    html += "var sx=x*ledSize,sy=y*ledSize+gap;";
    html += "var onCol=rgb565ToHex(ledColor);";
    html += "var surCol=rgb565ToHex(surroundColor);";
    html += "if(style===0){";
    html += "tftCtx.fillStyle=lit?onCol:'#000';";
    html += "tftCtx.fillRect(sx,sy,ledSize,ledSize);";
    html += "}else{";
    html += "tftCtx.fillStyle='#000';tftCtx.fillRect(sx,sy,ledSize,ledSize);";
    html += "if(lit){";
    html += "tftCtx.fillStyle=surCol;";
    html += "tftCtx.beginPath();tftCtx.arc(sx+ledSize/2,sy+ledSize/2,ledSize/2-1,0,Math.PI*2);tftCtx.fill();";
    html += "tftCtx.fillStyle=onCol;";
    html += "tftCtx.beginPath();tftCtx.arc(sx+ledSize/2,sy+ledSize/2,ledSize/2-2,0,Math.PI*2);tftCtx.fill();";
    html += "}else{";
    html += "tftCtx.fillStyle='#180000';";
    html += "tftCtx.beginPath();tftCtx.arc(sx+ledSize/2,sy+ledSize/2,ledSize/2-2,0,Math.PI*2);tftCtx.fill();";
    html += "}}}";
    html += "function updateDisplay(){";
    html += "fetch('/api/display')";
    html += ".then(function(r){return r.json();})";
    html += ".then(function(d){";
    html += "if(!tftCtx)initCanvas();";
    html += "if(!tftCtx)return;";
    html += "var buf=d.buffer,w=d.width,style=d.style,ledCol=d.ledColor,surCol=d.surroundColor;";
    html += "for(var row=0;row<2;row++){";
    html += "for(var x=0;x<32;x++){";
    html += "var byteVal=buf[x+row*32];";
    html += "for(var bit=0;bit<8;bit++){";
    html += "var y=row*8+bit;";
    html += "var lit=(byteVal&(1<<bit))!==0;";
    html += "drawLED(x,y,lit,style,ledCol,surCol);";
    html += "}}}})";
    html += ".catch(function(e){console.log('Display update failed:',e);});";
    html += "}";
    html += "setInterval(updateDisplay,500);";
    html += "setTimeout(function(){initCanvas();updateDisplay();},200);";
    html += "</script>";
    html += "</head><body>";
    html += "<div class='header'><h1>ESP32 CYD LED Matrix Clock</h1></div>";

    html += "<div class='time-display'>";
    html += "<h2>Current Time & Environment</h2>";
    html += "<div class='clock' id='clock'>--:--:--</div>";
    html += "<div class='date' id='date'>--/--/----</div>";
    html += "</div>";

    html += "<div class='tft-mirror'>";
    html += "<h2>TFT Display Mirror</h2>";
    html += "<div class='canvas-container'><canvas id='tftCanvas'></canvas></div>";
    html += "<p class='tft-label'>Live display - Updates every 500ms | 32×16 LED Matrix</p>";
    html += "<p style='color:#888;font-size:12px;margin:4px 0 0 0;'>💡 Tip: If seconds are truncated, adjust LED Size or Spacing below</p>";
    html += "</div>";

    if (sensorAvailable) {
      int tempDisplay = useFahrenheit ? (temperature * 9 / 5 + 32) : temperature;

      String tempIcon = "🌡️";
      String tempColor = "#FFA500";
      if (temperature >= 30) {
        tempIcon = "🔥";
        tempColor = "#FF4444";
      } else if (temperature >= 25) {
        tempIcon = "☀️";
        tempColor = "#FFB347";
      } else if (temperature >= 20) {
        tempIcon = "🌤️";
        tempColor = "#FFD700";
      } else if (temperature >= 15) {
        tempIcon = "⛅";
        tempColor = "#87CEEB";
      } else if (temperature >= 10) {
        tempIcon = "☁️";
        tempColor = "#B0C4DE";
      } else if (temperature >= 5) {
        tempIcon = "🌧️";
        tempColor = "#4682B4";
      } else {
        tempIcon = "❄️";
        tempColor = "#00CED1";
      }

      String humidityIcon = "💧";
      String humidityColor = "#4A90E2";
      if (humidity >= 70) {
        humidityIcon = "💦";
        humidityColor = "#1E90FF";
      } else if (humidity <= 30) {
        humidityIcon = "🏜️";
        humidityColor = "#DEB887";
      }

      html += "<div class='environment'>";
      html += "<div class='env-grid'>";

      html += "<div class='env-item'>";
      html += "<span class='env-icon'>" + tempIcon + "</span>";
      html += "<div class='env-value' style='color:" + tempColor + ";text-shadow:0 0 20px " + tempColor + "44;'>" + String(tempDisplay) + (useFahrenheit ? "°F" : "°C") + "</div>";
      html += "<div class='env-label'>Temperature</div>";
      html += "</div>";

      html += "<div class='env-item'>";
      html += "<span class='env-icon'>" + humidityIcon + "</span>";
      html += "<div class='env-value' style='color:" + humidityColor + ";text-shadow:0 0 20px " + humidityColor + "44;'>" + String(humidity) + "%</div>";
      html += "<div class='env-label'>Humidity</div>";
      html += "</div>";

#ifdef USE_BME280
      // Only show pressure for BME280 sensor
      html += "<div class='env-item'>";
      html += "<span class='env-icon'>🌍</span>";
      html += "<div class='env-value' style='color:#9370DB;text-shadow:0 0 20px #9370DB44;'>" + String(pressure) + "</div>";
      html += "<div class='env-label'>Pressure (hPa)</div>";
      html += "</div>";
#endif

      html += "</div></div>";
    }

    html += "<div class='card'><h2>Settings</h2>";
    html += "<button onclick=\"location.href='/temperature?mode=toggle'\" style='margin:0;'>Toggle °C/°F</button>";
    html += "</div>";
    
    html += "<div class='card'><h2>Display Style</h2>";
    html += "<p style='margin:4px 0;'>Current Style: " + String(displayStyle == 0 ? "Default (Blocks)" : "Realistic (LEDs)") + "</p>";
    html += "<button onclick=\"location.href='/style?mode=toggle'\">Toggle Style</button><br>";

    html += "<p style='margin:8px 0 4px 0;'>Display Rotation: " + String(displayRotation == 1 ? "Normal" : "Flipped 180°") + "</p>";
    html += "<button onclick=\"location.href='/rotation?mode=toggle'\">Flip Display</button><br>";

    html += "<p style='margin:8px 0 4px 0;'>LED Color:</p>";
    html += "<select id='ledcolor' onchange=\"location.href='/style?ledcolor='+this.value\">";
    html += "<option value='0'" + String(ledOnColor == COLOR_RED ? " selected" : "") + ">Red</option>";
    html += "<option value='1'" + String(ledOnColor == COLOR_GREEN ? " selected" : "") + ">Green</option>";
    html += "<option value='2'" + String(ledOnColor == COLOR_BLUE ? " selected" : "") + ">Blue</option>";
    html += "<option value='3'" + String(ledOnColor == COLOR_YELLOW ? " selected" : "") + ">Yellow</option>";
    html += "<option value='4'" + String(ledOnColor == COLOR_CYAN ? " selected" : "") + ">Cyan</option>";
    html += "<option value='5'" + String(ledOnColor == COLOR_MAGENTA ? " selected" : "") + ">Magenta</option>";
    html += "<option value='6'" + String(ledOnColor == COLOR_WHITE ? " selected" : "") + ">White</option>";
    html += "<option value='7'" + String(ledOnColor == COLOR_ORANGE ? " selected" : "") + ">Orange</option>";
    html += "</select><br>";
    
    html += "<p style='margin:8px 0 4px 0;'>Surround Color:</p>";
    html += "<select id='surroundcolor' onchange=\"location.href='/style?surroundcolor='+this.value\">";
    html += "<option value='0'" + String(ledSurroundColor == COLOR_WHITE ? " selected" : "") + ">White</option>";
    html += "<option value='1'" + String(ledSurroundColor == COLOR_LIGHT_GRAY ? " selected" : "") + ">Light Gray</option>";
    html += "<option value='2'" + String(ledSurroundColor == COLOR_DARK_GRAY ? " selected" : "") + ">Dark Gray</option>";
    html += "<option value='3'" + String(ledSurroundColor == COLOR_RED ? " selected" : "") + ">Red</option>";
    html += "<option value='4'" + String(ledSurroundColor == COLOR_GREEN ? " selected" : "") + ">Green</option>";
    html += "<option value='5'" + String(ledSurroundColor == COLOR_BLUE ? " selected" : "") + ">Blue</option>";
    html += "<option value='6'" + String(ledSurroundColor == COLOR_YELLOW ? " selected" : "") + ">Yellow</option>";
    html += "<option value='7'" + String(ledSurroundColor == ledOnColor ? " selected" : "") + ">Match LED Color</option>";
    html += "</select><br>";

    html += "<p style='margin:8px 0 4px 0;'>LED Size: <span id='ledSizeValue'>" + String(ledSize) + "</span> pixels</p>";
    html += "<input type='range' min='4' max='12' value='" + String(ledSize) + "' ";
    html += "oninput=\"document.getElementById('ledSizeValue').textContent=this.value\" ";
    html += "onchange=\"location.href='/style?ledsize='+this.value\" ";
    html += "style='width:100%;'>";
    html += "<small style='color:#888;display:block;margin:2px 0 8px 0;'>Range: 4-12 pixels (default: 9)</small>";

    html += "<p style='margin:8px 0 4px 0;'>LED Spacing: <span id='ledSpacingValue'>" + String(ledSpacing) + "</span> pixels</p>";
    html += "<input type='range' min='0' max='3' value='" + String(ledSpacing) + "' ";
    html += "oninput=\"document.getElementById('ledSpacingValue').textContent=this.value\" ";
    html += "onchange=\"location.href='/style?ledspacing='+this.value\" ";
    html += "style='width:100%;'>";
    html += "<small style='color:#888;display:block;margin:2px 0 8px 0;'>Range: 0-3 pixels (default: 1)</small>";

    html += "<p style='margin:8px 0 4px 0;'>Mode Switch Interval: <span id='modeSwitchIntervalValue'>" + String(modeSwitchInterval) + "</span> seconds</p>";
    html += "<input type='range' min='1' max='60' value='" + String(modeSwitchInterval) + "' ";
    html += "oninput=\"document.getElementById('modeSwitchIntervalValue').textContent=this.value\" ";
    html += "onchange=\"location.href='/modeinterval?seconds='+this.value\" ";
    html += "style='width:100%;'>";
    html += "<small style='color:#888;display:block;margin:2px 0;'>Range: 1-60 seconds (default: 5)</small>";
    html += "</div>";
    
    html += "<div class='card'><h2>Timezone & Time Format</h2>";
    html += "<p style='margin:8px 0 4px 0;'>Current Timezone: " + String(timezones[currentTimezone].name) + "</p>";
    html += "<select id='tz' onchange=\"location.href='/timezone?tz='+this.value\" style='margin-bottom:8px;'>";

    // Australia & Oceania (0-11)
    html += "<optgroup label='Australia & Oceania'>";
    for (int i = 0; i <= 11; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // North America (12-22)
    html += "<optgroup label='North America'>";
    for (int i = 12; i <= 22; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // South America (23-28)
    html += "<optgroup label='South America'>";
    for (int i = 23; i <= 28; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // Western Europe (29-39)
    html += "<optgroup label='Western Europe'>";
    for (int i = 29; i <= 39; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // Northern Europe (40-43)
    html += "<optgroup label='Northern Europe'>";
    for (int i = 40; i <= 43; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // Central & Eastern Europe (44-51)
    html += "<optgroup label='Central & Eastern Europe'>";
    for (int i = 44; i <= 51; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // Middle East (52-56)
    html += "<optgroup label='Middle East'>";
    for (int i = 52; i <= 56; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // South Asia (57-63)
    html += "<optgroup label='South Asia'>";
    for (int i = 57; i <= 63; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // Southeast Asia (64-70)
    html += "<optgroup label='Southeast Asia'>";
    for (int i = 64; i <= 70; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // East Asia (71-76)
    html += "<optgroup label='East Asia'>";
    for (int i = 71; i <= 76; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // Central Asia (77-79)
    html += "<optgroup label='Central Asia'>";
    for (int i = 77; i <= 79; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // Caucasus (80-82)
    html += "<optgroup label='Caucasus'>";
    for (int i = 80; i <= 82; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    // Africa (83-86)
    html += "<optgroup label='Africa'>";
    for (int i = 83; i <= 86; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</optgroup>";

    html += "</select><br>";

    html += "<p style='margin:8px 0 4px 0;'>Time Format: " + String(use24HourFormat ? "24-Hour" : "12-Hour") + "</p>";
    html += "<button onclick=\"location.href='/timeformat?mode=toggle'\">Toggle 12/24 Hour</button><br>";

    html += "<p style='margin:8px 0 4px 0;'>Leading Zero: " + String(showLeadingZero ? "ON (01:23)" : "OFF (1:23)") + "</p>";
    html += "<button onclick=\"location.href='/leadingzero?mode=toggle'\">Toggle Leading Zero</button><br>";

    html += "<p style='margin:8px 0 4px 0;'>Date Format:</p>";
    html += "<select id='dateformat' onchange=\"location.href='/dateformat?format='+this.value\">";
    html += "<option value='0'" + String(dateFormat == 0 ? " selected" : "") + ">DD/MM/YY (08/01/26)</option>";
    html += "<option value='1'" + String(dateFormat == 1 ? " selected" : "") + ">MM/DD/YY (01/08/26)</option>";
    html += "<option value='2'" + String(dateFormat == 2 ? " selected" : "") + ">YYYY-MM-DD (2026-01-08)</option>";
    html += "<option value='3'" + String(dateFormat == 3 ? " selected" : "") + ">DD.MM.YYYY (08.01.2026)</option>";
    html += "<option value='4'" + String(dateFormat == 4 ? " selected" : "") + ">MM.DD.YYYY (01.08.2026)</option>";
    html += "</select>";
    html += "</div>";
    
    html += "<div class='card'><h2>System</h2>";
    html += "<p style='margin:4px 0;'>Board: ESP32 CYD (ESP32-2432S028R)</p>";

    // Sensor information inline
    if (sensorAvailable) {
      html += "<p style='margin:4px 0;'>Sensor: <strong style='color:#50C878;'>" + String(sensorType) + "</strong>";
#ifdef USE_BME280
      html += " (Temp/Humid/Press, 0x76/77)</p>";
#elif defined(USE_SHT3X)
      html += " (Temp/Humid, 0x44/45)</p>";
#elif defined(USE_HTU21D)
      html += " (Temp/Humid, 0x40)</p>";
#endif
    } else {
      html += "<p style='margin:4px 0;'>Sensor: <span style='color:#FFA500;'>Not detected</span></p>";
    }

    html += "<div style='margin:6px 0;'>";
    html += "<span class='status-pill'>OTA ENABLED</span>";
    html += "<span class='status-subtext'>Use CYD-Clock.local or the device IP on port 3232 for wireless uploads</span>";
    html += "</div>";
    html += "<div class='note'>OTA uploads require the password set in code and in platformio.ini (--auth). Default is CYD_OTA_2024—update both together if you change it.</div>";
    html += "<p style='margin:4px 0;'>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p style='margin:4px 0;'>Uptime: " + String(millis() / 1000) + "s</p>";
    html += "<p style='margin:4px 0;'>Free Heap: " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "<button onclick=\"if(confirm('Reset WiFi?'))location.href='/reset'\" style='margin-top:8px;'>Reset WiFi</button>";
    html += "</div>";

    // Footer panel
    html += "<div class='footer'>";
    html += "<div class='footer-content'>";
    html += "<a href='https://github.com/anthonyjclarke/CYD_TFT_RetroClock' target='_blank' class='footer-link'>GitHub</a>";
    html += "<span class='footer-separator'>|</span>";
    html += "<a href='https://bsky.app/profile/anthonyjclarke.bsky.social' target='_blank' class='footer-link'>Bluesky</a>";
    html += "</div>";
    html += "<div class='footer-content'>";
    html += "<span style='color:#aaa;font-size:clamp(13px,3.5vw,15px);'>Built with</span>";
    html += "<span class='footer-heart'>❤️</span>";
    html += "<span style='color:#aaa;font-size:clamp(13px,3.5vw,15px);'>by Anthony Clarke</span>";
    html += "</div>";
    html += "<div class='footer-credit'>";
    html += "Based on the original ESP8266 TFT LED Matrix Clock by ";
    html += "<a href='https://www.youtube.com/watch?v=2wJOdi0xzas&t=32s' target='_blank'>@cbm80amiga</a>";
    html += "</div>";
    html += "</div>";

    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  
  // API endpoints
  server.on("/api/time", []() {
    String json = "{\"hours\":" + String(hours24) + ",\"minutes\":" + String(minutes) +
                  ",\"seconds\":" + String(seconds) + ",\"day\":" + String(day) +
                  ",\"month\":" + String(month) + ",\"year\":" + String(year) +
                  ",\"use24hour\":" + String(use24HourFormat ? "true" : "false") +
                  ",\"dateFormat\":" + String(dateFormat) + "}";
    server.send(200, "application/json", json);
  });
  
  // Display buffer API endpoint
  server.on("/api/display", []() {
    String json = "{\"buffer\":[";
    for (int i = 0; i < LINE_WIDTH * DISPLAY_ROWS; i++) {
      json += String(scr[i]);
      if (i < LINE_WIDTH * DISPLAY_ROWS - 1) json += ",";
    }
    json += "],\"width\":" + String(LINE_WIDTH) + ",\"height\":" + String(TOTAL_HEIGHT);
    json += ",\"style\":" + String(displayStyle);
    json += ",\"ledColor\":" + String(ledOnColor);
    json += ",\"surroundColor\":" + String(ledSurroundColor) + "}";
    server.send(200, "application/json", json);
  });
  
  // Temperature unit toggle
  server.on("/temperature", []() {
    if (server.hasArg("mode") && server.arg("mode") == "toggle") {
      useFahrenheit = !useFahrenheit;
      settingsChanged = true;
      DEBUG_SETTINGS(Serial.printf("=== SETTINGS CHANGED ===\nTemperature unit: %s\n", useFahrenheit ? "Fahrenheit" : "Celsius"));
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  // Time format toggle
  server.on("/timeformat", []() {
    if (server.hasArg("mode") && server.arg("mode") == "toggle") {
      use24HourFormat = !use24HourFormat;
      settingsChanged = true;
      DEBUG_SETTINGS(Serial.printf("=== SETTINGS CHANGED ===\nTime format: %s\nLeading zero: %s\n",
        use24HourFormat ? "24-hour" : "12-hour",
        showLeadingZero ? "ON" : "OFF"));
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  // Leading zero toggle
  server.on("/leadingzero", []() {
    if (server.hasArg("mode") && server.arg("mode") == "toggle") {
      showLeadingZero = !showLeadingZero;
      settingsChanged = true;
      DEBUG_SETTINGS(Serial.printf("=== SETTINGS CHANGED ===\nLeading zero: %s\nTime format: %s\n",
        showLeadingZero ? "ON" : "OFF",
        use24HourFormat ? "24-hour" : "12-hour"));
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  // Date format selector
  server.on("/dateformat", []() {
    if (server.hasArg("format")) {
      int newFormat = server.arg("format").toInt();
      if (newFormat >= 0 && newFormat <= 4) {
        dateFormat = newFormat;
        settingsChanged = true;
        const char* formatNames[] = {"DD/MM/YY", "MM/DD/YY", "YYYY-MM-DD", "DD.MM.YYYY", "MM.DD.YYYY"};
        DEBUG_SETTINGS(Serial.printf("=== SETTINGS CHANGED ===\nDate format: %s\n", formatNames[dateFormat]));

        // Redraw display with new format
        tft.fillScreen(BG_COLOR);
        forceFullRedraw = true;
        switch (currentMode) {
          case 0: displayTimeAndTemp(); break;
          case 1: displayTimeLarge(); break;
          case 2: displayTimeAndDate(); break;
        }
        refreshAll();
      }
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  // Mode switch interval
  server.on("/modeinterval", []() {
    if (server.hasArg("seconds")) {
      int newInterval = server.arg("seconds").toInt();
      if (newInterval >= 1 && newInterval <= 60) {
        modeSwitchInterval = newInterval;
        settingsChanged = true;
        DEBUG_SETTINGS(Serial.printf("=== SETTINGS CHANGED ===\nMode switch interval: %d seconds\n", modeSwitchInterval));
      }
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  // Timezone selection
  server.on("/timezone", []() {
    if (server.hasArg("tz")) {
      int tz = server.arg("tz").toInt();
      if (tz >= 0 && tz < numTimezones) {
        currentTimezone = tz;
        syncNTP();
        settingsChanged = true;
        DEBUG_SETTINGS(Serial.printf("=== SETTINGS CHANGED ===\nTimezone: %s\n", timezones[currentTimezone].name));
      }
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  
  // Style settings
  server.on("/style", []() {
    bool changed = false;
    String changeDetails = "=== SETTINGS CHANGED ===\n";

    if (server.hasArg("mode") && server.arg("mode") == "toggle") {
      displayStyle = (displayStyle + 1) % 2;
      changed = true;
      changeDetails += "Display style: " + String(displayStyle == 0 ? "Default (Blocks)" : "Realistic (LEDs)") + "\n";
    }

    if (server.hasArg("ledcolor")) {
      int colorIdx = server.arg("ledcolor").toInt();
      String colorName;
      switch(colorIdx) {
        case 0: ledOnColor = COLOR_RED; colorName = "Red"; break;
        case 1: ledOnColor = COLOR_GREEN; colorName = "Green"; break;
        case 2: ledOnColor = COLOR_BLUE; colorName = "Blue"; break;
        case 3: ledOnColor = COLOR_YELLOW; colorName = "Yellow"; break;
        case 4: ledOnColor = COLOR_CYAN; colorName = "Cyan"; break;
        case 5: ledOnColor = COLOR_MAGENTA; colorName = "Magenta"; break;
        case 6: ledOnColor = COLOR_WHITE; colorName = "White"; break;
        case 7: ledOnColor = COLOR_ORANGE; colorName = "Orange"; break;
        default: ledOnColor = COLOR_RED; colorName = "Red";
      }
      ledOffColor = ledOnColor >> 3;

      if (surroundMatchesLED) {
        ledSurroundColor = ledOnColor;
      }

      changed = true;
      changeDetails += "LED color: " + colorName + "\n";
    }

    if (server.hasArg("surroundcolor")) {
      int colorIdx = server.arg("surroundcolor").toInt();
      String colorName;
      switch(colorIdx) {
        case 0:
          ledSurroundColor = COLOR_WHITE;
          surroundMatchesLED = false;
          colorName = "White";
          break;
        case 1:
          ledSurroundColor = COLOR_LIGHT_GRAY;
          surroundMatchesLED = false;
          colorName = "Light Gray";
          break;
        case 2:
          ledSurroundColor = COLOR_DARK_GRAY;
          surroundMatchesLED = false;
          colorName = "Dark Gray";
          break;
        case 3:
          ledSurroundColor = COLOR_RED;
          surroundMatchesLED = false;
          colorName = "Red";
          break;
        case 4:
          ledSurroundColor = COLOR_GREEN;
          surroundMatchesLED = false;
          colorName = "Green";
          break;
        case 5:
          ledSurroundColor = COLOR_BLUE;
          surroundMatchesLED = false;
          colorName = "Blue";
          break;
        case 6:
          ledSurroundColor = COLOR_YELLOW;
          surroundMatchesLED = false;
          colorName = "Yellow";
          break;
        case 7:
          ledSurroundColor = ledOnColor;
          surroundMatchesLED = true;
          colorName = "Match LED";
          break;
        default:
          ledSurroundColor = COLOR_WHITE;
          surroundMatchesLED = false;
          colorName = "White";
      }
      changed = true;
      changeDetails += "Surround color: " + colorName + "\n";
    }

    if (server.hasArg("ledsize")) {
      int newSize = server.arg("ledsize").toInt();
      if (newSize >= 4 && newSize <= 12) {  // Reasonable range: 4-12 pixels
        ledSize = newSize;
        changed = true;
        changeDetails += "LED size: " + String(ledSize) + "px\n";
      }
    }

    if (server.hasArg("ledspacing")) {
      int newSpacing = server.arg("ledspacing").toInt();
      if (newSpacing >= 0 && newSpacing <= 3) {  // Reasonable range: 0-3 pixels
        ledSpacing = newSpacing;
        changed = true;
        changeDetails += "LED spacing: " + String(ledSpacing) + "px\n";
      }
    }

    if (changed) {
      settingsChanged = true;
      DEBUG_SETTINGS(Serial.print(changeDetails.c_str()));

      tft.fillScreen(BG_COLOR);
      forceFullRedraw = true;

      switch (currentMode) {
        case 0: displayTimeAndTemp(); break;
        case 1: displayTimeLarge(); break;
        case 2: displayTimeAndDate(); break;
      }
      refreshAll();
    }
    
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  // Display rotation toggle
  server.on("/rotation", []() {
    if (server.hasArg("mode") && server.arg("mode") == "toggle") {
      displayRotation = (displayRotation == 1) ? 3 : 1;
      settingsChanged = true;
      DEBUG_SETTINGS(Serial.printf("=== SETTINGS CHANGED ===\nDisplay rotation: %s\n",
        displayRotation == 1 ? "Normal" : "Flipped 180°"));

      // Update TFT rotation and redraw
      tft.setRotation(displayRotation);
      tft.fillScreen(BG_COLOR);
      forceFullRedraw = true;

      switch (currentMode) {
        case 0: displayTimeAndTemp(); break;
        case 1: displayTimeLarge(); break;
        case 2: displayTimeAndDate(); break;
      }
      refreshAll();
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });

  // Reset WiFi
  server.on("/reset", []() {
    server.send(200, "text/html", 
      "<html><body><h1>WiFi Reset</h1><p>WiFi settings cleared. Device will restart...</p></body></html>");
    delay(1000);
    wifiManager.resetSettings();
    ESP.restart();
  });
  
  // Handle not found
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not Found");
  });
  
  server.begin();
  DEBUG(Serial.println("\n=== Web Server Started ==="));
  DEBUG(Serial.print("Server running at http://"));
  DEBUG(Serial.println(WiFi.localIP()));
  DEBUG(Serial.println("Available endpoints: /, /api/time, /api/display, /temperature, /timezone, /style, /timeformat, /reset"));
  DEBUG(Serial.println("\nTry accessing the web server from your browser now!"));
  DEBUG(Serial.println("If you can't connect, check:"));
  DEBUG(Serial.println("  1. Your device is on the same WiFi network"));
  DEBUG(Serial.println("  2. Firewall isn't blocking port 80"));
  DEBUG(Serial.println("  3. Router isn't isolating WiFi clients"));
}

// ======================== FORWARD DECLARATIONS ========================
void configModeCallback(WiFiManager* myWiFiManager);

// ======================== SETUP ========================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  DEBUG(Serial.println("\n\n╔════════════════════════════════════════╗"));
  DEBUG(Serial.println("║   ESP32 CYD TFT Matrix Clock v3.5      ║"));
  DEBUG(Serial.println("║   Cheap Yellow Display Edition         ║"));
  DEBUG(Serial.println("╚════════════════════════════════════════╝\n"));

  // Initialize boot button
  pinMode(BOOT_BTN_PIN, INPUT_PULLUP);

  // Initialize RGB LED pins
  pinMode(LED_R_PIN, OUTPUT);
  pinMode(LED_G_PIN, OUTPUT);
  pinMode(LED_B_PIN, OUTPUT);
  setRGBLed(false, false, false);  // All off initially

  // Check if BOOT button is pressed during startup to reset WiFi
  bool resetWiFi = false;
  if (digitalRead(BOOT_BTN_PIN) == LOW) {
    DEBUG(Serial.println("\n⚠️  BOOT button pressed - checking for WiFi reset..."));
    setRGBLed(1, 1, 0);  // Yellow LED to indicate button detected

    // Wait for button to be held for 3 seconds
    unsigned long pressStart = millis();
    while (digitalRead(BOOT_BTN_PIN) == LOW && (millis() - pressStart) < 3000) {
      delay(100);
    }

    if (millis() - pressStart >= 3000) {
      resetWiFi = true;
      DEBUG(Serial.println("✓ BOOT button held for 3 seconds - WiFi will be reset!"));
      setRGBLed(1, 0, 0);  // Red LED
      delay(500);
    } else {
      DEBUG(Serial.println("✗ Button released too early - WiFi will not be reset"));
      setRGBLed(false, false, false);
    }
  }

  // Flash blue to indicate startup
  flashRGBLed(0, 0, 1, 500);
  
  // Initialize TFT display
  initTFT();

  showMessage("INIT");

  // Show reset WiFi message if button was held
  if (resetWiFi) {
    delay(500);
    showMessage("RESET");
    delay(500);
    showMessage("WIFI");
    delay(1000);
  }
  
  // Test sensor
  sensorAvailable = testSensor();
  if (sensorAvailable) {
    updateSensorData();
    // Flash green if sensor found
    flashRGBLed(0, 1, 0);
  } else {
    // Flash yellow (red+green) if no sensor
    flashRGBLed(1, 1, 0);
  }
  
  // WiFi setup
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setTimeout(180);

  // Reset WiFi if button was held during boot
  if (resetWiFi) {
    DEBUG(Serial.println("\n🔄 Resetting WiFi credentials..."));
    showMessage("WIFI RST");
    wifiManager.resetSettings();
    delay(1000);
    DEBUG(Serial.println("✓ WiFi credentials cleared!"));
  }

  showMessage("WIFI");
  // Set LED to blue during WiFi connection
  setRGBLed(0, 0, 1);
  
  if (!wifiManager.autoConnect("CYD_Clock_Setup")) {
    DEBUG(Serial.println("Failed to connect, restarting..."));
    // Flash red on failure
    for (int i = 0; i < 5; i++) {
      flashRGBLed(1, 0, 0, 200);
      delay(200);
    }
    ESP.restart();
  }
  
  setRGBLed(false, false, false);  // Turn off LED

  // Ensure we're in station mode after WiFiManager
  WiFi.mode(WIFI_STA);

  // Wait a moment for WiFi to stabilize
  delay(500);

  // Verify connection stability
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 10) {
    delay(500);
    retries++;
    DEBUG(Serial.print("."));
  }
  DEBUG(Serial.println());

  if (WiFi.status() != WL_CONNECTED) {
    DEBUG(Serial.println("WiFi connection lost! Restarting..."));
    ESP.restart();
  }

  DEBUG(Serial.println("\n=== WiFi Connected ==="));
  DEBUG(Serial.print("SSID: "));
  DEBUG(Serial.println(WiFi.SSID()));
  DEBUG(Serial.print("IP Address: "));
  DEBUG(Serial.println(WiFi.localIP()));
  DEBUG(Serial.print("Gateway: "));
  DEBUG(Serial.println(WiFi.gatewayIP()));
  DEBUG(Serial.print("Subnet Mask: "));
  DEBUG(Serial.println(WiFi.subnetMask()));
  DEBUG(Serial.print("DNS: "));
  DEBUG(Serial.println(WiFi.dnsIP()));
  DEBUG(Serial.print("Signal Strength (RSSI): "));
  DEBUG(Serial.print(WiFi.RSSI()));
  DEBUG(Serial.println(" dBm"));
  DEBUG(Serial.print("WiFi Mode: "));
  DEBUG(Serial.println(WiFi.getMode() == WIFI_STA ? "STA" : "Other"));

  showMessage("WIFI OK");
  flashRGBLed(0, 1, 0);  // Green flash for success
  delay(1000);

  // Display IP address split across two rows
  String ipStr = WiFi.localIP().toString();
  showIPAddress(ipStr.c_str());
  delay(2500);  // Pause for 2.5 seconds to allow user to see IP address
  
  // Sync time
  showMessage("NTP");
  syncNTP();
  showMessage("TIME OK");
  delay(1000);
  
  // Start web server
  setupWebServer();

  // Setup OTA (Over-The-Air) updates
  ArduinoOTA.setHostname("CYD-Clock");
  ArduinoOTA.setPassword("CYD_OTA_2024");  // Change this to a secure password

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    DEBUG(Serial.println("OTA Update Start: " + type));
    showMessage("OTA");
  });

  ArduinoOTA.onEnd([]() {
    DEBUG(Serial.println("\nOTA Update Complete"));
    showMessage("OTA OK");
    delay(1000);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    unsigned int percent = (progress / (total / 100));
    DEBUG(Serial.printf("OTA Progress: %u%%\r", percent));
    if (percent % 10 == 0) {  // Update display every 10%
      char msg[16];
      sprintf(msg, "OTA %d%%", percent);
      showMessage(msg);
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG(Serial.printf("OTA Error[%u]: ", error));
    if (error == OTA_AUTH_ERROR) DEBUG(Serial.println("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) DEBUG(Serial.println("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) DEBUG(Serial.println("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) DEBUG(Serial.println("Receive Failed"));
    else if (error == OTA_END_ERROR) DEBUG(Serial.println("End Failed"));
    showMessage("OTA ERR");
    flashRGBLed(1, 0, 0);  // Red flash for error
    delay(2000);
  });

  ArduinoOTA.begin();
  DEBUG(Serial.println("OTA Ready - Hostname: CYD-Clock"));
  DEBUG(Serial.print("OTA IP Address: "));
  DEBUG(Serial.println(WiFi.localIP()));

  showMessage("READY");
  delay(1000);

  // Initialize display with current time
  clearScreen();
  tft.fillScreen(BG_COLOR);
  updateTime();

  lastNTPSync = millis();
  lastSensorUpdate = millis();
  lastStatusPrint = millis();
  lastModeSwitch = millis();
}

// ======================== MAIN LOOP ========================

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();

  // Handle web server clients - this is critical for ESP32
  server.handleClient();
  yield();  // Allow background tasks to run
  
  unsigned long now = millis();
  
  // Update time
  updateTime();

  // Update sensor data
  if (sensorAvailable && now - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL) {
    updateSensorData();
    lastSensorUpdate = now;
  }
  
  // Periodic NTP sync
  if (now - lastNTPSync >= NTP_SYNC_INTERVAL) {
    syncNTP();
    lastNTPSync = now;
  }
  
  // Print status
  if (now - lastStatusPrint >= STATUS_PRINT_INTERVAL) {
    DEBUG(Serial.printf("Time: %02d:%02d | Date: %02d/%02d/%04d | Temp: %d°C | Hum: %d%% | Heap: %d\n",
                        hours24, minutes, day, month, year, temperature, humidity, ESP.getFreeHeap()));
    DEBUG(Serial.printf("WiFi Status: %s | IP: %s | RSSI: %d dBm\n",
                        WiFi.status() == WL_CONNECTED ? "Connected" : "DISCONNECTED",
                        WiFi.localIP().toString().c_str(),
                        WiFi.RSSI()));
    lastStatusPrint = now;
  }

  // Check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    DEBUG(Serial.println("WiFi disconnected! Attempting to reconnect..."));
    WiFi.reconnect();
    delay(5000);
    if (WiFi.status() != WL_CONNECTED) {
      DEBUG(Serial.println("Reconnection failed. Restarting..."));
      ESP.restart();
    }
  }
  
  delay(1);  // Minimal delay - ESP32 handles this well
}

// ======================== HELPER FUNCTIONS ========================

void configModeCallback(WiFiManager* myWiFiManager) {
  DEBUG(Serial.println("\n=== WiFi Config Mode ==="));
  DEBUG(Serial.println("Connect to AP: CYD_Clock_Setup"));
  DEBUG(Serial.print("Config portal IP: "));
  DEBUG(Serial.println(WiFi.softAPIP()));
  
  // Set LED to purple (red+blue) for config mode
  setRGBLed(1, 0, 1);
  
  showMessage("SETUP AP");
}
