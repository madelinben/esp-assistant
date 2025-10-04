/**
 * @file HomeAssistantController.cpp
 * @brief Implementation of HomeAssistantController
 */

#include "controllers/apps/home-assistant/HomeAssistantController.h"
#include "models/home-assistant/HomeAssistantDevice.h"

// Home Assistant API endpoints
static const char* ENDPOINT_STATES = "/api/states";
static const char* ENDPOINT_SERVICES = "/api/services";
static const char* ENDPOINT_CONFIG = "/api/config";

HomeAssistantController& HomeAssistantController::getInstance() {
    static HomeAssistantController instance;
    return instance;
}

HomeAssistantController::HomeAssistantController()
    : m_serverUrl("")
    , m_accessToken("")
    , m_authenticated(false)
    , m_initialized(false)
    , m_devices(nullptr)
    , m_deviceCount(0)
    , m_maxDevices(50)
    , m_lastPollTime(0)
    , m_pollInterval(10000) {  // Poll every 10 seconds
    
    // Allocate device buffer
    m_devices = new HomeAssistantDevice[m_maxDevices];
}

HomeAssistantController::~HomeAssistantController() {
    if (m_devices) {
        delete[] m_devices;
    }
}

bool HomeAssistantController::init() {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[HomeAssistantController] Initializing...");

    // Load server URL and token from database
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (currentUser) {
        String serverUrl = DatabaseService::getInstance().getSetting(currentUser->getId(), "ha_server_url", "");
        String token = DatabaseService::getInstance().getToken(currentUser->getId(), "home-assistant");
        
        if (serverUrl.length() > 0 && token.length() > 0) {
            DEBUG_PRINTLN("[HomeAssistantController] Found saved credentials");
            setServerUrl(serverUrl);
            setAccessToken(token);
            authenticate();
        } else {
            DEBUG_PRINTLN("[HomeAssistantController] No saved credentials found");
        }
    }

    m_initialized = true;
    DEBUG_PRINTLN("[HomeAssistantController] Initialized");
    return true;
}

void HomeAssistantController::update() {
    if (!m_initialized || !m_authenticated) {
        return;
    }

    // Check if it's time to poll
    uint32_t currentTime = millis();
    if (currentTime - m_lastPollTime >= m_pollInterval) {
        m_lastPollTime = currentTime;
        
        // Fetch device states
        DEBUG_PRINTLN("[HomeAssistantController] Polling for updates...");
        fetchDevices();
    }
}

void HomeAssistantController::setServerUrl(const String& url) {
    m_serverUrl = url;
    
    // Save to database
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (currentUser) {
        DatabaseService::getInstance().saveSetting(currentUser->getId(), "ha_server_url", url);
    }
}

void HomeAssistantController::setAccessToken(const String& token) {
    m_accessToken = token;
    
    // Save token to database
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (currentUser) {
        DatabaseService::getInstance().saveToken(currentUser->getId(), "home-assistant", token, "access_token");
    }
}

String HomeAssistantController::getServerUrl() {
    return m_serverUrl;
}

String HomeAssistantController::getAccessToken() {
    return m_accessToken;
}

bool HomeAssistantController::authenticate() {
    if (m_serverUrl.length() == 0 || m_accessToken.length() == 0) {
        DEBUG_PRINTLN("[HomeAssistantController] ERROR: No server URL or token set");
        return false;
    }

    if (!NetworkService::getInstance().isConnected()) {
        DEBUG_PRINTLN("[HomeAssistantController] ERROR: Not connected to network");
        return false;
    }

    DEBUG_PRINTLN("[HomeAssistantController] Authenticating...");

    String response;
    if (!makeAPIRequest(ENDPOINT_CONFIG, "GET", "", response)) {
        DEBUG_PRINTLN("[HomeAssistantController] Authentication failed");
        m_authenticated = false;
        return false;
    }

    // Parse response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        DEBUG_PRINTF("[HomeAssistantController] JSON parse error: %s\n", error.c_str());
        m_authenticated = false;
        return false;
    }

    // Check for valid config response
    if (!doc.containsKey("version")) {
        DEBUG_PRINTLN("[HomeAssistantController] Invalid config response");
        m_authenticated = false;
        return false;
    }

    m_authenticated = true;
    DEBUG_PRINTLN("[HomeAssistantController] Authenticated successfully!");
    DEBUG_PRINTF("[HomeAssistantController] HA Version: %s\n", doc["version"].as<const char*>());

    // Fetch initial device list
    fetchDevices();

    return true;
}

bool HomeAssistantController::isAuthenticated() {
    return m_authenticated;
}

bool HomeAssistantController::fetchDevices() {
    if (!m_authenticated) {
        DEBUG_PRINTLN("[HomeAssistantController] Not authenticated");
        return false;
    }

    DEBUG_PRINTLN("[HomeAssistantController] Fetching devices...");

    String response;
    if (!makeAPIRequest(ENDPOINT_STATES, "GET", "", response)) {
        DEBUG_PRINTLN("[HomeAssistantController] Failed to fetch devices");
        return false;
    }

    // Parse and store devices
    parseDevices(response);

    DEBUG_PRINTF("[HomeAssistantController] Fetched %d devices\n", m_deviceCount);
    return true;
}

int HomeAssistantController::getDevicesByType(HomeAssistantDeviceType type, HomeAssistantDevice* devices, int maxDevices) {
    if (!devices || maxDevices <= 0) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < m_deviceCount && count < maxDevices; i++) {
        if (m_devices[i].type == type) {
            devices[count++] = m_devices[i];
        }
    }

    return count;
}

int HomeAssistantController::getAllDevices(HomeAssistantDevice* devices, int maxDevices) {
    if (!devices || maxDevices <= 0) {
        return 0;
    }

    int count = (m_deviceCount < maxDevices) ? m_deviceCount : maxDevices;
    
    for (int i = 0; i < count; i++) {
        devices[i] = m_devices[i];
    }

    return count;
}

HomeAssistantDevice* HomeAssistantController::getDevice(const String& entityId) {
    for (int i = 0; i < m_deviceCount; i++) {
        if (m_devices[i].entityId == entityId) {
            return &m_devices[i];
        }
    }
    return nullptr;
}

bool HomeAssistantController::turnOn(const String& entityId) {
    String domain = entityId.substring(0, entityId.indexOf('.'));
    return callService(domain, "turn_on", entityId);
}

bool HomeAssistantController::turnOff(const String& entityId) {
    String domain = entityId.substring(0, entityId.indexOf('.'));
    return callService(domain, "turn_off", entityId);
}

bool HomeAssistantController::toggle(const String& entityId) {
    String domain = entityId.substring(0, entityId.indexOf('.'));
    return callService(domain, "toggle", entityId);
}

bool HomeAssistantController::setBrightness(const String& entityId, uint8_t brightness) {
    DynamicJsonDocument doc(256);
    doc["brightness"] = brightness;
    
    String data;
    serializeJson(doc, data);
    
    return callService("light", "turn_on", entityId, data);
}

bool HomeAssistantController::setColor(const String& entityId, uint8_t r, uint8_t g, uint8_t b) {
    DynamicJsonDocument doc(256);
    JsonArray color = doc.createNestedArray("rgb_color");
    color.add(r);
    color.add(g);
    color.add(b);
    
    String data;
    serializeJson(doc, data);
    
    return callService("light", "turn_on", entityId, data);
}

bool HomeAssistantController::setColorTemp(const String& entityId, uint16_t colorTemp) {
    DynamicJsonDocument doc(256);
    doc["color_temp"] = colorTemp;
    
    String data;
    serializeJson(doc, data);
    
    return callService("light", "turn_on", entityId, data);
}

bool HomeAssistantController::activateScene(const String& sceneId) {
    return callService("scene", "turn_on", sceneId);
}

bool HomeAssistantController::setMediaPlayerVolume(const String& entityId, float volume) {
    if (!m_authenticated) {
        DEBUG_PRINTLN("[HomeAssistantController] Not authenticated");
        return false;
    }

    // Clamp volume to 0.0-1.0
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    DEBUG_PRINTF("[HomeAssistantController] Set media player volume: %.2f\n", volume);

    DynamicJsonDocument doc(256);
    doc["volume_level"] = volume;
    
    String data;
    serializeJson(doc, data);
    
    return callService("media_player", "volume_set", entityId, data);
}

bool HomeAssistantController::getDeviceState(const String& entityId, HomeAssistantDevice& device) {
    HomeAssistantDevice* foundDevice = getDevice(entityId);
    if (foundDevice) {
        device = *foundDevice;
        return true;
    }
    return false;
}

bool HomeAssistantController::callService(const String& domain, const String& service, const String& entityId, const String& data) {
    if (!m_authenticated) {
        DEBUG_PRINTLN("[HomeAssistantController] Not authenticated");
        return false;
    }

    DEBUG_PRINTF("[HomeAssistantController] Calling service: %s.%s for %s\n", 
                 domain.c_str(), service.c_str(), entityId.c_str());

    // Build endpoint
    String endpoint = String(ENDPOINT_SERVICES) + "/" + domain + "/" + service;

    // Build payload
    DynamicJsonDocument doc(512);
    doc["entity_id"] = entityId;
    
    // Merge additional data if provided
    if (data.length() > 0) {
        DynamicJsonDocument dataDoc(256);
        deserializeJson(dataDoc, data);
        JsonObject dataObj = dataDoc.as<JsonObject>();
        for (JsonPair kv : dataObj) {
            doc[kv.key()] = kv.value();
        }
    }

    String payload;
    serializeJson(doc, payload);

    String response;
    if (!makeAPIRequest(endpoint, "POST", payload, response)) {
        DEBUG_PRINTLN("[HomeAssistantController] Service call failed");
        return false;
    }

    DEBUG_PRINTLN("[HomeAssistantController] Service call successful");
    return true;
}

bool HomeAssistantController::makeAPIRequest(const String& endpoint, const String& method, const String& payload, String& response) {
    if (!NetworkService::getInstance().isConnected()) {
        DEBUG_PRINTLN("[HomeAssistantController] Not connected to network");
        return false;
    }

    HTTPClient http;
    String url = m_serverUrl + endpoint;

    DEBUG_PRINTF("[HomeAssistantController] %s %s\n", method.c_str(), url.c_str());

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + m_accessToken);
    http.addHeader("Content-Type", "application/json");

    int httpCode;
    if (method == "POST") {
        httpCode = http.POST(payload);
    } else {
        httpCode = http.GET();
    }

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
            response = http.getString();
            http.end();
            return true;
        } else {
            DEBUG_PRINTF("[HomeAssistantController] HTTP error: %d\n", httpCode);
        }
    } else {
        DEBUG_PRINTF("[HomeAssistantController] Request failed: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    return false;
}

void HomeAssistantController::parseDevices(const String& jsonResponse) {
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error) {
        DEBUG_PRINTF("[HomeAssistantController] JSON parse error: %s\n", error.c_str());
        return;
    }

    // Reset device count
    m_deviceCount = 0;

    // Parse states array
    JsonArray states = doc.as<JsonArray>();
    for (JsonObject state : states) {
        if (m_deviceCount >= m_maxDevices) {
            DEBUG_PRINTLN("[HomeAssistantController] Device buffer full");
            break;
        }

        String entityId = state["entity_id"].as<String>();
        String stateStr = state["state"].as<String>();
        JsonObject attributes = state["attributes"];

        // Create device
        HomeAssistantDevice& device = m_devices[m_deviceCount];
        device.entityId = entityId;
        device.friendlyName = attributes["friendly_name"].as<String>();
        device.state = (stateStr == "on") ? HomeAssistantDeviceState::ON : 
                      (stateStr == "off") ? HomeAssistantDeviceState::OFF :
                      (stateStr == "unavailable") ? HomeAssistantDeviceState::UNAVAILABLE :
                      HomeAssistantDeviceState::UNKNOWN;
        device.type = getDeviceTypeFromEntityId(entityId);
        
        // Parse type-specific attributes
        if (device.type == HomeAssistantDeviceType::LIGHT) {
            device.brightness = attributes["brightness"].as<uint8_t>();
            device.hasColor = attributes.containsKey("rgb_color");
            device.hasColorTemp = attributes.containsKey("color_temp");
        }

        m_deviceCount++;
    }
}

void HomeAssistantController::updateDeviceState(const String& entityId, const String& state, const JsonObject& attributes) {
    HomeAssistantDevice* device = getDevice(entityId);
    if (device) {
        device->state = (state == "on") ? HomeAssistantDeviceState::ON : 
                       (state == "off") ? HomeAssistantDeviceState::OFF :
                       HomeAssistantDeviceState::UNKNOWN;
        
        if (device->type == HomeAssistantDeviceType::LIGHT) {
            device->brightness = attributes["brightness"].as<uint8_t>();
        }
    }
}

HomeAssistantDeviceType HomeAssistantController::getDeviceTypeFromEntityId(const String& entityId) {
    if (entityId.startsWith("light.")) return HomeAssistantDeviceType::LIGHT;
    if (entityId.startsWith("switch.")) return HomeAssistantDeviceType::SWITCH;
    if (entityId.startsWith("sensor.")) return HomeAssistantDeviceType::SENSOR;
    if (entityId.startsWith("climate.")) return HomeAssistantDeviceType::CLIMATE;
    if (entityId.startsWith("media_player.")) return HomeAssistantDeviceType::MEDIA_PLAYER;
    if (entityId.startsWith("cover.")) return HomeAssistantDeviceType::COVER;
    if (entityId.startsWith("fan.")) return HomeAssistantDeviceType::FAN;
    if (entityId.startsWith("lock.")) return HomeAssistantDeviceType::LOCK;
    return HomeAssistantDeviceType::UNKNOWN;
}

