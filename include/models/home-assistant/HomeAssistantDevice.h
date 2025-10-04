/**
 * @file HomeAssistantDevice.h
 * @brief Home Assistant device data model - MVC Model Layer
 * 
 * Data structure for Home Assistant devices.
 * Part of MVC architecture - Model layer.
 */

#ifndef HOME_ASSISTANT_DEVICE_H
#define HOME_ASSISTANT_DEVICE_H

#include <Arduino.h>

// Forward declarations
enum class HomeAssistantDeviceType;
enum class HomeAssistantDeviceState;

/**
 * @struct HomeAssistantDevice
 * @brief Home Assistant device data
 */
struct HomeAssistantDevice {
    String entityId;
    String friendlyName;
    HomeAssistantDeviceType type;
    HomeAssistantDeviceState state;
    
    // Light-specific attributes
    uint8_t brightness;      // 0-255
    uint8_t colorR;          // 0-255
    uint8_t colorG;          // 0-255
    uint8_t colorB;          // 0-255
    uint16_t colorTemp;      // Color temperature in mireds
    bool hasColor;
    bool hasColorTemp;
    
    // Climate-specific attributes
    float temperature;
    float targetTemperature;
    String hvacMode;
    
    // Media player-specific attributes
    String mediaTitle;
    String mediaArtist;
    float volume;
    
    // Sensor-specific attributes
    String sensorValue;
    String unit;
    
    HomeAssistantDevice()
        : entityId("")
        , friendlyName("")
        , brightness(0)
        , colorR(0)
        , colorG(0)
        , colorB(0)
        , colorTemp(0)
        , hasColor(false)
        , hasColorTemp(false)
        , temperature(0.0f)
        , targetTemperature(0.0f)
        , hvacMode("")
        , mediaTitle("")
        , mediaArtist("")
        , volume(0.0f)
        , sensorValue("")
        , unit("") {}
};

#endif // HOME_ASSISTANT_DEVICE_H


