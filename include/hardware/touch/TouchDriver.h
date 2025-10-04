/**
 * @file TouchDriver.h
 * @brief Hardware driver for CST816T I2C touch controller
 * 
 * Low-level touch input driver for ESP32-S3 Touch LCD.
 * Part of Hardware Abstraction Layer (HAL).
 */

#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include "config/Config.h"

/**
 * @struct TouchData
 * @brief Raw touch data from hardware
 */
struct TouchData {
    int16_t x;
    int16_t y;
    bool touched;
    uint8_t gesture;
};

/**
 * @class TouchDriver
 * @brief Singleton hardware driver for touch input
 * 
 * Handles low-level touch controller communication via I2C.
 * This is the HAL layer - Controllers process touch events.
 */
class TouchDriver {
public:
    /**
     * @brief Get singleton instance
     */
    static TouchDriver& getInstance();

    /**
     * @brief Initialize touch hardware
     * @return true if successful
     */
    bool init();

    /**
     * @brief Read current touch data
     * @param data Output touch data
     * @return true if touch detected
     */
    bool read(TouchData& data);

    /**
     * @brief Check if interrupt pin is active
     */
    bool hasInterrupt();

private:
    TouchDriver();
    ~TouchDriver();
    TouchDriver(const TouchDriver&) = delete;
    TouchDriver& operator=(const TouchDriver&) = delete;

    bool readRegister(uint8_t reg, uint8_t* data, uint8_t len);
    bool m_initialized;
};

#endif // TOUCH_DRIVER_H


