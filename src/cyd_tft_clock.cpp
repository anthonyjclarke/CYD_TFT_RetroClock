/*
 * ESP32 CYD TFT Matrix Clock
 * Author: Refactored for ESP32 CYD by Anthony Clarke
 * Board: ESP32-2432S028R (Cheap Yellow Display)
 * Display: Built-in 2.8" ILI9341 320x240 TFT
 *
 * This version is refactored from the ESP8266 TFT LED Matrix Clock
 * to run on the ESP32 Cheap Yellow Display (CYD) board.
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
 * ======================== CHANGELOG ========================
 * 28th December 2025 - Version 3.0 (CYD Port):
 *   - Complete refactor from ESP8266 to ESP32 CYD board
 *   - Changed WiFi libraries to ESP32 variants
 *   - Updated pin definitions for CYD hardware
 *   - Added RGB LED support (GPIO 4, 16, 17)
 *   - Updated I2C pins for CYD extended GPIO connector
 *   - Maintained all original functionality and web interface
 *   - Updated TFT_eSPI configuration for CYD pinout
 *   - Larger display (320x240) allows better LED matrix simulation
 *
 * Previous Version History:
 * 19th December 2025 - Version 2.2:
 *   - Added TFT Display Mirror feature on web page
 *   - Canvas-based LED matrix rendering in browser
 *
 * 18th December 2025 - Version 2.1:
 *   - Fixed Mode 2 (Time+Date) leading zero removal
 *   - Redesigned web interface with modern dark theme
 *   - Added dynamic temperature/humidity icons
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

// ======================== LIBRARIES ========================
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
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

// I2C for BME280 sensor (using extended GPIO connector CN1)
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
#define LED_SIZE          10      // Size of each simulated LED pixel (10 * 32 = 320px, fits in 320)
#define LED_SPACING       1      // No spacing between LEDs

// Color definitions (RGB565 format)
#define LED_COLOR         0xF800 // Red color for LEDs
#define BG_COLOR          0x0000 // Black background
#define LED_OFF_COLOR     0x2000 // Dim red for "off" LEDs

// Calculate display dimensions for centering
// When LED_SPACING = 0: LED_SIZE * count
// Plus 4-pixel gap between the two matrix rows (authentic spacing)
#define DISPLAY_WIDTH     (LED_SIZE * TOTAL_WIDTH)   // 288 pixels (fits in 320)
#define DISPLAY_HEIGHT    (LED_SIZE * TOTAL_HEIGHT + 4)  // 148 pixels (fits in 240)

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
#define STATUS_PRINT_INTERVAL        10000  // Print status every 10s

// ======================== DEBUG CONFIGURATION ========================
#define DEBUG_ENABLED 1

// ======================== DISPLAY OPTIMIZATION ========================
#define BRIGHTNESS_BOOST 1  // Set to 1 for maximum brightness
#define FAST_REFRESH 1      // Set to 1 to only redraw changed pixels

#if DEBUG_ENABLED
  #define DEBUG(x) x
#else
  #define DEBUG(x)
#endif

// ======================== DISPLAY OBJECT ========================
TFT_eSPI tft = TFT_eSPI();  // TFT_eSPI uses configuration from User_Setup.h

// ======================== DISPLAY BUFFER ========================
// Virtual screen buffer matching original LED matrix structure
byte scr[LINE_WIDTH * DISPLAY_ROWS]; // 32 columns × 2 rows = 64 bytes

// ======================== GLOBAL OBJECTS ========================
WebServer server(80);
WiFiManager wifiManager;
Adafruit_BME280 bme280;

// ======================== FONT INCLUDES ========================
#include "fonts.h"
#include "timezones.h"

// ======================== TIME VARIABLES ========================
int hours = 0, minutes = 0, seconds = 0;
int hours24 = 0;  // 24-hour format
int day = 1, month = 1, year = 2025;
int lastSecond = -1;
bool use24HourFormat = false;  // Default to 12-hour format

// ======================== SENSOR VARIABLES ========================
bool sensorAvailable = false;
int temperature = 0;
int humidity = 0;
int pressure = 0;
bool useFahrenheit = false;

// ======================== TIMING VARIABLES ========================
unsigned long lastSensorUpdate = 0;
unsigned long lastNTPSync = 0;
unsigned long lastStatusPrint = 0;

// ======================== DISPLAY STYLE VARIABLES ========================
int displayStyle = DEFAULT_DISPLAY_STYLE;  // 0=Default, 1=Realistic
uint16_t ledOnColor = COLOR_RED;           // Color for lit LEDs
uint16_t ledSurroundColor = COLOR_DARK_GRAY; // Dark gray for authentic MAX7219 look
uint16_t ledOffColor = 0x2000;             // Color for unlit LEDs (dim)
bool surroundMatchesLED = false;           // Track if surround should match LED color
bool forceFullRedraw = false;              // Flag to force immediate complete redraw

// ======================== DISPLAY MODES ========================
int currentMode = 0; // 0=Time+Temp, 1=Time Large, 2=Time+Date
unsigned long lastModeSwitch = 0;
#define MODE_SWITCH_INTERVAL 5000

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
  tft.setRotation(1);  // Rotation 1 = landscape mode (320x240) - adjust if needed
  DEBUG(Serial.printf("TFT_eSPI initialized, rotation set to 1\n"));

  // Small delay after initialization
  delay(100);

  // Check actual dimensions
  DEBUG(Serial.printf("TFT reports dimensions: %d x %d\n", tft.width(), tft.height()));

  tft.fillScreen(BG_COLOR);

  // Calculate display dimensions
  int displayWidth = tft.width();
  int displayHeight = tft.height();
  int offsetX = ((displayWidth - DISPLAY_WIDTH) / 2) > 0 ? ((displayWidth - DISPLAY_WIDTH) / 2) : 0;
  int offsetY = ((displayHeight - DISPLAY_HEIGHT) / 2) > 0 ? ((displayHeight - DISPLAY_HEIGHT) / 2) : 0;
  
  DEBUG(Serial.printf("TFT Display initialized: %dx%d\n", displayWidth, displayHeight));
  DEBUG(Serial.printf("LED Matrix area: %dx%d at offset (%d,%d)\n", 
        DISPLAY_WIDTH, DISPLAY_HEIGHT, offsetX, offsetY));
  
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
  int offsetX = ((tft.width() - DISPLAY_WIDTH) / 2) > 0 ? ((tft.width() - DISPLAY_WIDTH) / 2) : 0;
  int offsetY = ((tft.height() - DISPLAY_HEIGHT) / 2) > 0 ? ((tft.height() - DISPLAY_HEIGHT) / 2) : 0;
  
  // Add extra gap between matrix rows (after row 7, before row 8)
  int matrixGap = (y >= 8) ? 4 : 0;  // 4-pixel gap between matrix rows
  
  int screenX = offsetX + x * LED_SIZE;
  int screenY = offsetY + y * LED_SIZE + matrixGap;
  
  if (displayStyle == 0) {
    // ========== DEFAULT STYLE: Solid square blocks ==========
    uint16_t color = lit ? ledOnColor : BG_COLOR;
    tft.fillRect(screenX, screenY, LED_SIZE, LED_SIZE, color);
  } 
  else {
    // ========== REALISTIC STYLE: Circular LED with surround ==========
    int center = LED_SIZE / 2;
    int ledRadius = (LED_SIZE - 2) / 2;  // Leave 1px border
    
    if (!lit) {
      // OFF LED: Show dark circle
      tft.fillRect(screenX, screenY, LED_SIZE, LED_SIZE, BG_COLOR);
      
      uint16_t offHousing = dimRGB565(ledSurroundColor, 7);
      uint16_t offLED = 0x1800;  // Very dark red
      
      for (int py = 1; py < LED_SIZE - 1; py++) {
        for (int px = 1; px < LED_SIZE - 1; px++) {
          int dx = (px * 2 - LED_SIZE + 1);
          int dy = (py * 2 - LED_SIZE + 1);
          int distSq = dx * dx + dy * dy;
          int threshInner = (LED_SIZE - 4) * (LED_SIZE - 4);
          int threshOuter = (LED_SIZE - 2) * (LED_SIZE - 2);
          
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
      tft.fillRect(screenX, screenY, LED_SIZE, LED_SIZE, ledSurroundColor);

      for (int py = 0; py < LED_SIZE; py++) {
        for (int px = 0; px < LED_SIZE; px++) {
          int dx = (px * 2 - LED_SIZE + 1);
          int dy = (py * 2 - LED_SIZE + 1);
          int distSq = dx * dx + dy * dy;

          uint16_t pixelColor;
          int threshCore = (LED_SIZE - 6) * (LED_SIZE - 6);
          int threshBody = (LED_SIZE - 2) * (LED_SIZE - 2);
          int threshSurround = LED_SIZE * LED_SIZE;

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
  bool canShowSeconds = true;
  
  if (use24HourFormat && hours24 >= 10) {
    canShowSeconds = false;
  }
  
  // Hours
  sprintf(buf, "%d", displayHours);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x8rn);
    if (*(p+1)) x++;
  }
  
  // Colon
  if (showDots) {
    x += drawCharWithY(x, 0, ':', digits5x8rn);
    x += 1;
  } else {
    x += 2;
  }
  
  // Minutes
  sprintf(buf, "%02d", minutes);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x8rn);
    if (*(p+1)) x++;
  }
  
  // Seconds
  if (canShowSeconds) {
    x++;
    sprintf(buf, "%02d", seconds);
    if (x + 7 <= LINE_WIDTH) {
      for (const char* p = buf; *p; p++) {
        if (x < LINE_WIDTH - 3) {
          x += drawCharWithY(x, 0, *p, digits3x5);
          if (*(p+1) && x < LINE_WIDTH) x++;
        }
      }
    }
  }
  
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
  int x = (displayHours > 9) ? 0 : 3;
  
  // Draw hours
  sprintf(buf, "%d", displayHours);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x16rn);
    if (*(p+1)) x++;
  }
  
  // Draw colon
  if (showDots) {
    x += drawCharWithY(x, 0, ':', digits5x16rn);
  } else {
    x += 1;
  }
  
  // Draw minutes
  sprintf(buf, "%02d", minutes);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x16rn);
    if (*(p+1)) x++;
  }
  
  // Add seconds in small font
  x++;
  sprintf(buf, "%02d", seconds);
  for (const char* p = buf; *p; p++) {
    if (x < LINE_WIDTH - 3) {
      x += drawCharWithY(x, 0, *p, font3x7);
      if (*(p+1) && x < LINE_WIDTH - 3) x++;
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
  sprintf(buf, "%d", displayHours);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 0, *p, digits5x8rn);
    if (*(p+1)) x++;
  }
  
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
  x++;
  sprintf(buf, "%02d", seconds);
  for (const char* p = buf; *p; p++) {
    if (x < LINE_WIDTH - 3) {
      x += drawCharWithY(x, 0, *p, digits3x5);
      if (*(p+1) && x < LINE_WIDTH - 3) x++;
    }
  }
  
  // Bottom row: Date
  x = 2;
  sprintf(buf, "%02d/%02d/%02d", day, month, year % 100);
  for (const char* p = buf; *p; p++) {
    x += drawCharWithY(x, 1, *p, font3x7) + 1;
  }
}

// ======================== SENSOR FUNCTIONS ========================

bool testSensor() {
  Wire.begin(SDA_PIN, SCL_PIN);
  
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
  return true;
}

void updateSensorData() {
  if (!sensorAvailable) return;
  
  bme280.takeForcedMeasurement();
  float temp = bme280.readTemperature();
  float hum = bme280.readHumidity();
  float pres = bme280.readPressure() / 100.0F;
  
  if (!isnan(temp) && temp >= -50 && temp <= 100) {
    temperature = (int)round(temp);
  }
  
  if (!isnan(hum) && hum >= 0 && hum <= 100) {
    humidity = (int)round(hum);
  }
  
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
    DEBUG(Serial.printf("Display update - Mode: %d, Time: %02d:%02d:%02d\n", currentMode, hours24, minutes, seconds));
    switch (currentMode) {
      case 0: displayTimeAndTemp(); break;
      case 1: displayTimeLarge(); break;
      case 2: displayTimeAndDate(); break;
    }
    refreshAll();
  }
  
  // Auto-switch modes
  if (millis() - lastModeSwitch > MODE_SWITCH_INTERVAL) {
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
    html += "body{font-family:'Segoe UI',Arial,sans-serif;margin:0;padding:15px;background:#1a1a1a;color:#fff;max-width:1200px;margin:0 auto;}";
    html += ".header{text-align:center;margin-bottom:20px;}";
    html += "h1{color:#fff;font-size:clamp(20px,5vw,28px);font-weight:600;margin:0 0 30px 0;}";
    html += ".time-display{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:clamp(20px,5vw,40px);border-radius:15px;box-shadow:0 8px 32px rgba(0,0,0,0.3);margin-bottom:20px;}";
    html += ".time-display h2{color:#aaa;font-size:clamp(16px,4vw,20px);font-weight:400;margin:0 0 15px 0;text-align:left;}";
    html += ".clock{font-size:clamp(48px,15vw,120px);font-weight:700;text-align:center;margin:15px 0;font-family:'Courier New',monospace;color:#7CFC00;text-shadow:0 0 30px rgba(124,252,0,0.5);line-height:1.1;}";
    html += ".date{font-size:clamp(24px,7vw,48px);font-weight:600;text-align:center;margin:15px 0;font-family:'Courier New',monospace;color:#4A90E2;text-shadow:0 0 20px rgba(74,144,226,0.5);line-height:1.2;}";
    html += ".environment{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:clamp(20px,4vw,40px);border-radius:15px;box-shadow:0 8px 32px rgba(0,0,0,0.3);margin-bottom:20px;}";
    html += ".environment p{margin:10px 0;}";
    html += ".env-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:clamp(15px,3vw,30px);text-align:center;}";
    html += ".env-item{padding:clamp(15px,3vw,20px);background:rgba(255,255,255,0.05);border-radius:10px;transition:transform 0.2s;}";
    html += ".env-item:hover{transform:translateY(-5px);background:rgba(255,255,255,0.08);}";
    html += ".env-icon{font-size:clamp(40px,10vw,60px);margin-bottom:8px;display:block;}";
    html += ".env-value{font-size:clamp(24px,6vw,36px);font-weight:700;margin:8px 0;font-family:'Courier New',monospace;line-height:1.2;}";
    html += ".env-label{font-size:clamp(12px,3vw,16px);color:#aaa;text-transform:uppercase;letter-spacing:1px;}";
    html += ".card{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:clamp(15px,3vw,20px);margin:10px 0;border-radius:10px;box-shadow:0 4px 16px rgba(0,0,0,0.3);}";
    html += "h2{color:#aaa;border-bottom:2px solid #4CAF50;padding-bottom:5px;font-size:clamp(16px,4vw,18px);font-weight:500;margin-top:0;}";
    html += "button{background:#4CAF50;color:white;border:none;padding:10px 15px;cursor:pointer;border-radius:5px;margin:5px;font-size:clamp(12px,3vw,14px);white-space:nowrap;}";
    html += "button:hover{background:#45a049;}";
    html += "select{padding:8px;font-size:clamp(12px,3vw,14px);background:#1e1e1e;color:#fff;border:1px solid #444;border-radius:5px;width:100%;max-width:300px;}";
    html += "p{color:#ccc;font-size:clamp(13px,3vw,15px);line-height:1.6;}";
    html += "@media(max-width:768px){";
    html += ".env-grid{grid-template-columns:1fr;}";
    html += ".clock{font-size:clamp(40px,12vw,80px);}";
    html += ".date{font-size:clamp(20px,6vw,36px);}";
    html += "body{padding:10px;}";
    html += ".time-display,.environment,.card{padding:15px;}";
    html += "}";
    // TFT Display Mirror styles
    html += ".tft-mirror{background:linear-gradient(135deg,#2a2a2a,#1e1e1e);padding:clamp(15px,3vw,25px);border-radius:15px;box-shadow:0 8px 32px rgba(0,0,0,0.3);margin-bottom:20px;text-align:center;}";
    html += ".tft-mirror h2{color:#aaa;border-bottom:2px solid #E91E63;padding-bottom:5px;font-size:clamp(16px,4vw,18px);font-weight:500;margin-top:0;text-align:left;}";
    html += ".canvas-container{display:flex;justify-content:center;align-items:center;padding:15px;background:#000;border-radius:10px;margin-top:15px;}";
    html += "#tftCanvas{image-rendering:pixelated;image-rendering:crisp-edges;border-radius:5px;}";
    html += ".tft-label{color:#888;font-size:12px;margin-top:10px;}";
    html += "</style>";
    html += "<script>";
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
    html += "if(date){date.textContent=(d.day<10?'0':'')+d.day+'/'+(d.month<10?'0':'')+d.month+'/'+d.year;}";
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

      html += "<div class='env-item'>";
      html += "<span class='env-icon'>🌍</span>";
      html += "<div class='env-value' style='color:#9370DB;text-shadow:0 0 20px #9370DB44;'>" + String(pressure) + "</div>";
      html += "<div class='env-label'>Pressure (hPa)</div>";
      html += "</div>";

      html += "</div></div>";
    }
    
    html += "<div class='card'><h2>Settings</h2>";
    html += "<button onclick=\"location.href='/temperature?mode=toggle'\">Toggle °C/°F</button>";
    html += "</div>";
    
    html += "<div class='card'><h2>Display Style</h2>";
    html += "<p>Current Style: " + String(displayStyle == 0 ? "Default (Blocks)" : "Realistic (LEDs)") + "</p>";
    html += "<button onclick=\"location.href='/style?mode=toggle'\">Toggle Style</button><br><br>";
    
    html += "<p>LED Color:</p>";
    html += "<select id='ledcolor' onchange=\"location.href='/style?ledcolor='+this.value\">";
    html += "<option value='0'" + String(ledOnColor == COLOR_RED ? " selected" : "") + ">Red</option>";
    html += "<option value='1'" + String(ledOnColor == COLOR_GREEN ? " selected" : "") + ">Green</option>";
    html += "<option value='2'" + String(ledOnColor == COLOR_BLUE ? " selected" : "") + ">Blue</option>";
    html += "<option value='3'" + String(ledOnColor == COLOR_YELLOW ? " selected" : "") + ">Yellow</option>";
    html += "<option value='4'" + String(ledOnColor == COLOR_CYAN ? " selected" : "") + ">Cyan</option>";
    html += "<option value='5'" + String(ledOnColor == COLOR_MAGENTA ? " selected" : "") + ">Magenta</option>";
    html += "<option value='6'" + String(ledOnColor == COLOR_WHITE ? " selected" : "") + ">White</option>";
    html += "<option value='7'" + String(ledOnColor == COLOR_ORANGE ? " selected" : "") + ">Orange</option>";
    html += "</select><br><br>";
    
    html += "<p>Surround Color:</p>";
    html += "<select id='surroundcolor' onchange=\"location.href='/style?surroundcolor='+this.value\">";
    html += "<option value='0'" + String(ledSurroundColor == COLOR_WHITE ? " selected" : "") + ">White</option>";
    html += "<option value='1'" + String(ledSurroundColor == COLOR_LIGHT_GRAY ? " selected" : "") + ">Light Gray</option>";
    html += "<option value='2'" + String(ledSurroundColor == COLOR_DARK_GRAY ? " selected" : "") + ">Dark Gray</option>";
    html += "<option value='3'" + String(ledSurroundColor == COLOR_RED ? " selected" : "") + ">Red</option>";
    html += "<option value='4'" + String(ledSurroundColor == COLOR_GREEN ? " selected" : "") + ">Green</option>";
    html += "<option value='5'" + String(ledSurroundColor == COLOR_BLUE ? " selected" : "") + ">Blue</option>";
    html += "<option value='6'" + String(ledSurroundColor == COLOR_YELLOW ? " selected" : "") + ">Yellow</option>";
    html += "<option value='7'" + String(ledSurroundColor == ledOnColor ? " selected" : "") + ">Match LED Color</option>";
    html += "</select>";
    html += "</div>";
    
    html += "<div class='card'><h2>Timezone & Time Format</h2>";
    html += "<p>Current Timezone: " + String(timezones[currentTimezone].name) + "</p>";
    html += "<select id='tz' onchange=\"location.href='/timezone?tz='+this.value\">";
    for (int i = 0; i < numTimezones; i++) {
      html += "<option value='" + String(i) + "'" + (i == currentTimezone ? " selected" : "") + ">";
      html += timezones[i].name;
      html += "</option>";
    }
    html += "</select><br><br>";
    
    html += "<p>Time Format: " + String(use24HourFormat ? "24-Hour" : "12-Hour") + "</p>";
    html += "<button onclick=\"location.href='/timeformat?mode=toggle'\">Toggle 12/24 Hour</button>";
    if (use24HourFormat) {
      html += "<p style='color:#666;font-size:12px;margin-top:10px;'>⚠️ Note: In Time+Temp mode, seconds not displayed when hours ≥ 10</p>";
    }
    html += "</div>";
    
    html += "<div class='card'><h2>System</h2>";
    html += "<p>Board: ESP32 CYD (ESP32-2432S028R)</p>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Uptime: " + String(millis() / 1000) + "s</p>";
    html += "<p>Free Heap: " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "<button onclick=\"if(confirm('Reset WiFi?'))location.href='/reset'\">Reset WiFi</button>";
    html += "</div>";
    
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  
  // API endpoints
  server.on("/api/time", []() {
    String json = "{\"hours\":" + String(hours24) + ",\"minutes\":" + String(minutes) +
                  ",\"seconds\":" + String(seconds) + ",\"day\":" + String(day) +
                  ",\"month\":" + String(month) + ",\"year\":" + String(year) +
                  ",\"use24hour\":" + String(use24HourFormat ? "true" : "false") + "}";
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
      DEBUG(Serial.printf("Temperature unit changed to: %s\n", useFahrenheit ? "Fahrenheit" : "Celsius"));
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  
  // Time format toggle
  server.on("/timeformat", []() {
    if (server.hasArg("mode") && server.arg("mode") == "toggle") {
      use24HourFormat = !use24HourFormat;
      DEBUG(Serial.printf("Time format changed to: %s\n", use24HourFormat ? "24-hour" : "12-hour"));
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
        DEBUG(Serial.printf("Timezone changed to: %s\n", timezones[currentTimezone].name));
      }
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  
  // Style settings
  server.on("/style", []() {
    bool changed = false;
    
    if (server.hasArg("mode") && server.arg("mode") == "toggle") {
      displayStyle = (displayStyle + 1) % 2;
      changed = true;
      DEBUG(Serial.printf("Display style changed to: %d (%s)\n", 
                          displayStyle, displayStyle == 0 ? "Default" : "Realistic"));
    }
    
    if (server.hasArg("ledcolor")) {
      int colorIdx = server.arg("ledcolor").toInt();
      switch(colorIdx) {
        case 0: ledOnColor = COLOR_RED; break;
        case 1: ledOnColor = COLOR_GREEN; break;
        case 2: ledOnColor = COLOR_BLUE; break;
        case 3: ledOnColor = COLOR_YELLOW; break;
        case 4: ledOnColor = COLOR_CYAN; break;
        case 5: ledOnColor = COLOR_MAGENTA; break;
        case 6: ledOnColor = COLOR_WHITE; break;
        case 7: ledOnColor = COLOR_ORANGE; break;
        default: ledOnColor = COLOR_RED;
      }
      ledOffColor = ledOnColor >> 3;
      
      if (surroundMatchesLED) {
        ledSurroundColor = ledOnColor;
      }
      
      changed = true;
      DEBUG(Serial.printf("LED color changed to index: %d\n", colorIdx));
    }
    
    if (server.hasArg("surroundcolor")) {
      int colorIdx = server.arg("surroundcolor").toInt();
      switch(colorIdx) {
        case 0: 
          ledSurroundColor = COLOR_WHITE; 
          surroundMatchesLED = false;
          break;
        case 1: 
          ledSurroundColor = COLOR_LIGHT_GRAY; 
          surroundMatchesLED = false;
          break;
        case 2: 
          ledSurroundColor = COLOR_DARK_GRAY; 
          surroundMatchesLED = false;
          break;
        case 3: 
          ledSurroundColor = COLOR_RED; 
          surroundMatchesLED = false;
          break;
        case 4: 
          ledSurroundColor = COLOR_GREEN; 
          surroundMatchesLED = false;
          break;
        case 5: 
          ledSurroundColor = COLOR_BLUE; 
          surroundMatchesLED = false;
          break;
        case 6: 
          ledSurroundColor = COLOR_YELLOW; 
          surroundMatchesLED = false;
          break;
        case 7: 
          ledSurroundColor = ledOnColor;
          surroundMatchesLED = true;
          break;
        default: 
          ledSurroundColor = COLOR_WHITE;
          surroundMatchesLED = false;
      }
      changed = true;
      DEBUG(Serial.printf("Surround color changed to index: %d, match mode: %s\n", 
                          colorIdx, surroundMatchesLED ? "ON" : "OFF"));
    }
    
    if (changed) {
      tft.fillScreen(BG_COLOR);
      forceFullRedraw = true;
      
      switch (currentMode) {
        case 0: displayTimeAndTemp(); break;
        case 1: displayTimeLarge(); break;
        case 2: displayTimeAndDate(); break;
      }
      refreshAll();

      DEBUG(Serial.println("Style changed - immediate redraw complete"));
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
  DEBUG(Serial.println("║   ESP32 CYD TFT Matrix Clock v3.0      ║"));
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
