/**
 * @file TouchDriver.cpp
 * @brief Implementation of TouchDriver
 */

#include "hardware/touch/TouchDriver.h"

TouchDriver& TouchDriver::getInstance() {
    static TouchDriver instance;
    return instance;
}

TouchDriver::TouchDriver() 
    : m_initialized(false) {
}

TouchDriver::~TouchDriver() {
}

bool TouchDriver::init() {
    if (m_initialized) {
        return true;
    }

    // Initialize I2C
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(400000);  // 400kHz

    // Initialize interrupt pin
    pinMode(TOUCH_INT, INPUT);

    // Reset touch controller
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_RST, HIGH);
    delay(50);

    m_initialized = true;
    DEBUG_PRINTLN("[TouchDriver] Initialized successfully");
    return true;
}

bool TouchDriver::read(TouchData& data) {
    if (!m_initialized) {
        return false;
    }

    uint8_t buffer[6];
    if (!readRegister(0x01, buffer, 6)) {
        return false;
    }

    data.touched = (buffer[0] > 0);
    data.x = ((buffer[1] & 0x0F) << 8) | buffer[2];
    data.y = ((buffer[3] & 0x0F) << 8) | buffer[4];
    data.gesture = buffer[5];

    return data.touched;
}

bool TouchDriver::hasInterrupt() {
    return digitalRead(TOUCH_INT) == LOW;
}

bool TouchDriver::readRegister(uint8_t reg, uint8_t* data, uint8_t len) {
    Wire.beginTransmission(TOUCH_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) {
        return false;
    }

    Wire.requestFrom(TOUCH_I2C_ADDR, len);
    for (uint8_t i = 0; i < len; i++) {
        if (Wire.available()) {
            data[i] = Wire.read();
        } else {
            return false;
        }
    }

    return true;
}


