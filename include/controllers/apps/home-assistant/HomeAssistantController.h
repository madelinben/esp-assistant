/**
 * @file HomeAssistantController.h
 * @brief Home Assistant controller for smart home integration - MVC Controller Layer
 * 
 * Manages Home Assistant API interactions and device control.
 * Part of MVC architecture - Controller layer (app controller).
 */

#ifndef HOME_ASSISTANT_CONTROLLER_H
#define HOME_ASSISTANT_CONTROLLER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "services/DatabaseService.h"
#include "services/AuthService.h"
#include "services/NetworkService.h"
#include "models/home-assistant/HomeAssistantDevice.h"

/**
 * @enum HomeAssistantDeviceType
 * @brief Types of Home Assistant devices
 */
enum class HomeAssistantDeviceType {
    LIGHT,
    SWITCH,
    SENSOR,
    CLIMATE,
    MEDIA_PLAYER,
    COVER,
    FAN,
    LOCK,
    UNKNOWN
};

/**
 * @enum HomeAssistantDeviceState
 * @brief Device state
 */
enum class HomeAssistantDeviceState {
    ON,
    OFF,
    UNAVAILABLE,
    UNKNOWN
};

/**
 * @class HomeAssistantController
 * @brief Singleton controller for Home Assistant integration
 * 
 * Features:
 * - Device discovery and management
 * - Real-time state monitoring
 * - Device control (on/off, brightness, color)
 * - Scene activation
 * - Automation triggers
 * - API authentication
 * - WebSocket support (future)
 */
class HomeAssistantController {
public:
    /**
     * @brief Get singleton instance
     */
    static HomeAssistantController& getInstance();

    /**
     * @brief Initialize controller
     * @return true if successful
     */
    bool init();

    /**
     * @brief Update controller (poll for state changes)
     */
    void update();

    /**
     * @brief Set server URL
     * @param url Home Assistant server URL (e.g., http://homeassistant.local:8123)
     */
    void setServerUrl(const String& url);

    /**
     * @brief Set access token
     * @param token Long-lived access token
     */
    void setAccessToken(const String& token);

    /**
     * @brief Get server URL
     * @return Server URL
     */
    String getServerUrl();

    /**
     * @brief Get access token
     * @return Access token
     */
    String getAccessToken();

    /**
     * @brief Authenticate with Home Assistant
     * @return true if authenticated
     */
    bool authenticate();

    /**
     * @brief Check if authenticated
     * @return true if authenticated
     */
    bool isAuthenticated();

    /**
     * @brief Fetch all devices
     * @return true if successful
     */
    bool fetchDevices();

    /**
     * @brief Get devices by type
     * @param type Device type filter
     * @param devices Output array
     * @param maxDevices Maximum devices to return
     * @return Number of devices
     */
    int getDevicesByType(HomeAssistantDeviceType type, HomeAssistantDevice* devices, int maxDevices);

    /**
     * @brief Get all devices
     * @param devices Output array
     * @param maxDevices Maximum devices to return
     * @return Number of devices
     */
    int getAllDevices(HomeAssistantDevice* devices, int maxDevices);

    /**
     * @brief Get device by entity ID
     * @param entityId Entity ID (e.g., "light.living_room")
     * @return Pointer to device (or nullptr)
     */
    HomeAssistantDevice* getDevice(const String& entityId);

    /**
     * @brief Turn device on
     * @param entityId Entity ID
     * @return true if successful
     */
    bool turnOn(const String& entityId);

    /**
     * @brief Turn device off
     * @param entityId Entity ID
     * @return true if successful
     */
    bool turnOff(const String& entityId);

    /**
     * @brief Toggle device state
     * @param entityId Entity ID
     * @return true if successful
     */
    bool toggle(const String& entityId);

    /**
     * @brief Set light brightness
     * @param entityId Entity ID
     * @param brightness Brightness (0-255)
     * @return true if successful
     */
    bool setBrightness(const String& entityId, uint8_t brightness);

    /**
     * @brief Set light brightness (alias for setBrightness)
     * @param entityId Entity ID
     * @param brightness Brightness (0-255)
     * @return true if successful
     */
    bool setLightBrightness(const String& entityId, uint8_t brightness) { return setBrightness(entityId, brightness); }

    /**
     * @brief Set light color (RGB)
     * @param entityId Entity ID
     * @param r Red (0-255)
     * @param g Green (0-255)
     * @param b Blue (0-255)
     * @return true if successful
     */
    bool setColor(const String& entityId, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Set light color temperature
     * @param entityId Entity ID
     * @param colorTemp Color temperature in mireds
     * @return true if successful
     */
    bool setColorTemp(const String& entityId, uint16_t colorTemp);

    /**
     * @brief Activate a scene
     * @param sceneId Scene entity ID (e.g., "scene.movie_time")
     * @return true if successful
     */
    bool activateScene(const String& sceneId);

    /**
     * @brief Set media player volume
     * @param entityId Entity ID
     * @param volume Volume level (0.0-1.0)
     * @return true if successful
     */
    bool setMediaPlayerVolume(const String& entityId, float volume);

    /**
     * @brief Get device state by entity ID
     * @param entityId Entity ID
     * @param device Output device state
     * @return true if device found
     */
    bool getDeviceState(const String& entityId, HomeAssistantDevice& device);

    /**
     * @brief Call a service
     * @param domain Service domain (e.g., "light")
     * @param service Service name (e.g., "turn_on")
     * @param entityId Target entity ID
     * @param data Additional service data (JSON)
     * @return true if successful
     */
    bool callService(const String& domain, const String& service, const String& entityId, const String& data = "");

private:
    HomeAssistantController();
    ~HomeAssistantController();
    HomeAssistantController(const HomeAssistantController&) = delete;
    HomeAssistantController& operator=(const HomeAssistantController&) = delete;

    bool makeAPIRequest(const String& endpoint, const String& method, const String& payload, String& response);
    void parseDevices(const String& jsonResponse);
    void updateDeviceState(const String& entityId, const String& state, const JsonObject& attributes);
    HomeAssistantDeviceType getDeviceTypeFromEntityId(const String& entityId);

    String m_serverUrl;
    String m_accessToken;
    bool m_authenticated;
    bool m_initialized;

    HomeAssistantDevice* m_devices;
    int m_deviceCount;
    int m_maxDevices;

    uint32_t m_lastPollTime;
    uint32_t m_pollInterval;  // Polling interval in ms
};

#endif // HOME_ASSISTANT_CONTROLLER_H

