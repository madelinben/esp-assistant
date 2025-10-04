/**
 * @file Config.h
 * @brief Global configuration for ESP32-S3 Touch LCD Assistant
 * 
 * Contains hardware pin definitions, system constants, and configuration
 * parameters for the entire project.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

// Display Configuration (ST7789 via TFT_eSPI)
#define SCREEN_WIDTH        360
#define SCREEN_HEIGHT       360
#define SCREEN_RADIUS       180
#define SCREEN_CENTER_X     180
#define SCREEN_CENTER_Y     180

// Display Pins (configured in platformio.ini for TFT_eSPI)
// MOSI: GPIO 11, SCLK: GPIO 12, CS: GPIO 10
// DC: GPIO 8, RST: GPIO 14, BL: GPIO 9

// Touch Controller (CST816T - I2C)
#define TOUCH_SDA           17
#define TOUCH_SCL           18
#define TOUCH_RST           16
#define TOUCH_INT           7
#define TOUCH_I2C_ADDR      0x15

// Audio Configuration
#define I2S_DOUT            13
#define I2S_BCLK            36
#define I2S_LRC             35
#define MIC_DATA            39
#define MIC_CLK             40
#define SPEAKER_PIN         46

// SD Card Configuration
#define SD_CS               41
#define SD_MOSI             2
#define SD_MISO             42
#define SD_SCK              1

// Power Management
#define BATTERY_ADC         4
#define CHARGE_STATUS       15

// RTC (PCF85063)
#define RTC_I2C_ADDR        0x51

// GPIO Expander (TCA9554PWR)
#define GPIO_EXP_I2C_ADDR   0x20

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================

// Performance
#define TARGET_FPS          30
#define FRAME_TIME_MS       (1000 / TARGET_FPS)

// UI Configuration
#define BORDER_WIDTH        10
#define BORDER_COLOR        TFT_BLUE
#define NOTIFICATION_RADIUS 170

// Hexagonal Grid Configuration
#define HEX_ITEM_RADIUS     40
#define HEX_SPACING         10

// Touch Sensitivity
#define TOUCH_DEADZONE      5
#define DRAG_THRESHOLD      10
#define SWIPE_THRESHOLD     50
#define TAP_MAX_DURATION    300  // ms

// Database Configuration
#define DB_PATH             "/sd/assistant.db"
#define DB_ENCRYPTION_KEY   "CHANGE_ME_IN_PRODUCTION"  // TODO: Implement secure key storage

// Wi-Fi Configuration
#define WIFI_TIMEOUT_MS     10000

// Debug Configuration
#define DEBUG_ENABLED       1
#define SERIAL_BAUD         115200

#if DEBUG_ENABLED
    #define DEBUG_PRINT(x)    Serial.print(x)
    #define DEBUG_PRINTLN(x)  Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

// ============================================================================
// COLOR DEFINITIONS
// ============================================================================

// Custom color palette (RGB565 format)
#define COLOR_BACKGROUND    0x0000  // Black
#define COLOR_PRIMARY       0x001F  // Blue
#define COLOR_SECONDARY     0x7BEF  // Light Gray
#define COLOR_ACCENT        0xFD20  // Orange
#define COLOR_TEXT          0xFFFF  // White
#define COLOR_TEXT_DIM      0x7BEF  // Gray
#define COLOR_SUCCESS       0x07E0  // Green
#define COLOR_WARNING       0xFFE0  // Yellow
#define COLOR_ERROR         0xF800  // Red

// ============================================================================
// APP IDENTIFIERS
// ============================================================================

enum AppId {
    APP_NONE = 0,
    APP_SLACK,
    APP_SPOTIFY,
    APP_HOME_ASSISTANT,
    APP_AI_ASSISTANT,
    APP_WEATHER,
    APP_CALENDAR,
    APP_SETTINGS
};

// ============================================================================
// SYSTEM LIMITS
// ============================================================================

#define MAX_USERS           5
#define MAX_APPS_PER_USER   10
#define MAX_NOTIFICATIONS   20
#define MAX_GRID_ITEMS      19  // 1 center + 6 + 12 in hexagonal pattern

#endif // CONFIG_H


