/**
 * @file DisplayDriver.h
 * @brief Hardware driver for ST7789 360x360 circular display
 * 
 * Low-level display driver with double-buffering for ESP32-S3 Touch LCD.
 * Part of Hardware Abstraction Layer (HAL).
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <TFT_eSPI.h>
#include "config/Config.h"

/**
 * @class DisplayDriver
 * @brief Singleton hardware driver for display management
 * 
 * Handles low-level display operations including initialization,
 * double-buffering, and frame rendering. This is the HAL layer.
 */
class DisplayDriver {
public:
    /**
     * @brief Get singleton instance
     */
    static DisplayDriver& getInstance();

    /**
     * @brief Initialize display hardware
     * @return true if successful
     */
    bool init();

    /**
     * @brief Set display brightness
     * @param brightness Level 0-255
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief Get TFT instance for direct drawing
     */
    TFT_eSPI* getTFT() { return &m_tft; }

    /**
     * @brief Get sprite for double-buffering
     */
    TFT_eSprite* getSprite() { return m_sprite; }

    /**
     * @brief Swap buffers and display frame
     */
    void swapBuffers();

    /**
     * @brief Clear buffer
     */
    void clear(uint16_t color = 0x0000);

    /**
     * @brief Draw circular border around screen edge
     * @param color Border color
     * @param width Border width in pixels
     */
    void drawCircularBorder(uint16_t color = BORDER_COLOR, uint16_t width = BORDER_WIDTH);

    /**
     * @brief Draw curved text along circular path
     * @param text Text to draw
     * @param angleStart Starting angle (degrees, 0 = top)
     * @param radius Radius for text placement
     * @param color Text color
     */
    void drawCurvedText(const char* text, float angleStart, float radius, uint16_t color);

    /**
     * @brief Check if coordinate is inside circular display
     * @param x X coordinate
     * @param y Y coordinate
     * @return true if inside circle
     */
    bool isInsideCircle(int16_t x, int16_t y) const;

    /**
     * @brief Draw pixel only if inside circular bounds
     */
    void drawPixelClipped(int16_t x, int16_t y, uint16_t color);

    /**
     * @brief Fill circle clipped to display bounds
     */
    void fillCircleClipped(int16_t x, int16_t y, int16_t r, uint16_t color);

    // Screen constants
    const int WIDTH = SCREEN_WIDTH;
    const int HEIGHT = SCREEN_HEIGHT;
    const int RADIUS = SCREEN_RADIUS;

private:
    DisplayDriver();
    ~DisplayDriver();
    DisplayDriver(const DisplayDriver&) = delete;
    DisplayDriver& operator=(const DisplayDriver&) = delete;

    TFT_eSPI m_tft;
    TFT_eSprite* m_sprite;
    bool m_initialized;
};

#endif // DISPLAY_DRIVER_H

