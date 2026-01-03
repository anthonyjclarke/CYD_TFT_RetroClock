/*
 * User_Setup.h - TFT_eSPI Configuration for ESP32 Cheap Yellow Display (CYD)
 * 
 * Board: ESP32-2432S028R (Cheap Yellow Display)
 * Display: 2.8" ILI9341 320x240 TFT with resistive touchscreen
 * 
 * This configuration file is specifically designed for the CYD board.
 * The board uses HSPI for the TFT display and VSPI for the touchscreen.
 * 
 * Pin Configuration Reference:
 * TFT Display (HSPI):
 *   - MOSI: GPIO 13
 *   - MISO: GPIO 12
 *   - CLK:  GPIO 14
 *   - CS:   GPIO 15
 *   - DC:   GPIO 2
 *   - RST:  Not connected (-1)
 *   - BL:   GPIO 21 (Backlight)
 * 
 * Touchscreen (VSPI) - XPT2046:
 *   - T_IRQ:  GPIO 36
 *   - T_DIN:  GPIO 32 (MOSI)
 *   - T_OUT:  GPIO 39 (MISO)
 *   - T_CLK:  GPIO 25
 *   - T_CS:   GPIO 33
 * 
 * RGB LED (active low):
 *   - Red:   GPIO 4
 *   - Green: GPIO 16
 *   - Blue:  GPIO 17
 * 
 * I2C (for external sensors via extended GPIO):
 *   - SDA: GPIO 27
 *   - SCL: GPIO 22
 * 
 * Available GPIOs on extended connectors:
 *   - GPIO 35, GPIO 22, GPIO 21, GPIO 27
 */

#ifndef USER_SETUP_H
#define USER_SETUP_H

#define USER_SETUP_ID 303  // CYD ESP32-2432S028R

// =====================================================================
// Section 1: Driver Selection
// =====================================================================
// Select only ONE driver - comment out all others

// For most CYD boards (v1, v2) - ILI9341 driver
#define ILI9341_2_DRIVER    // Alternative ILI9341 driver with better compatibility

// For some newer CYD boards (v3, 2USB variant) - uncomment if ILI9341 doesn't work
// #define ST7789_DRIVER

// =====================================================================
// Section 2: TFT Display Pin Configuration (HSPI)
// =====================================================================

// TFT SPI Pins
#define TFT_MOSI  13    // SPI Master Out Slave In
#define TFT_MISO  12    // SPI Master In Slave Out (not used for display, but defined)
#define TFT_SCLK  14    // SPI Clock
#define TFT_CS    15    // Chip select
#define TFT_DC     2    // Data/Command control
#define TFT_RST   -1    // Reset not connected (tied to ESP32 EN/RST)

// Backlight control
#define TFT_BL    21    // Backlight pin (can use PWM for brightness control)
#define TFT_BACKLIGHT_ON HIGH  // HIGH = backlight on

// =====================================================================
// Section 3: Display Dimensions
// =====================================================================

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// =====================================================================
// Section 4: Fonts to Load
// =====================================================================

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font
#define LOAD_FONT2  // Font 2. Small 16 pixel high font
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font
#define LOAD_FONT6  // Font 6. Large 48 pixel font, only 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, only 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font, only 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts

// Optional: Smooth font support (uses more memory)
// #define SMOOTH_FONT

// =====================================================================
// Section 5: SPI Configuration
// =====================================================================

// Use ESP32 hardware SPI
#define SPI_FREQUENCY       40000000    // 40 MHz for TFT
#define SPI_READ_FREQUENCY  20000000    // 20 MHz for reading
#define SPI_TOUCH_FREQUENCY  2500000    // 2.5 MHz for touch

// =====================================================================
// Section 6: Color Order (if colors are wrong, try swapping)
// =====================================================================

// Uncomment if colors are inverted (blue/red swapped)
// #define TFT_RGB_ORDER TFT_BGR

// =====================================================================
// Section 7: Touchscreen Configuration (XPT2046)
// =====================================================================
// Note: Touchscreen pins are handled separately in code using XPT2046_Touchscreen library
// These defines are for reference and compatibility

#define TOUCH_CS  33     // Touchscreen chip select

// =====================================================================
// Section 8: Other Options
// =====================================================================

// Transactions support for simultaneous SPI device access
// #define SUPPORT_TRANSACTIONS

#endif // USER_SETUP_H
