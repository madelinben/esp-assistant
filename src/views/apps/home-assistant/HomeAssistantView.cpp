/**
 * @file HomeAssistantView.cpp
 * @brief Implementation of HomeAssistantView
 */

#include "views/apps/home-assistant/HomeAssistantView.h"
#include "hardware/display/DisplayDriver.h"

// Global callback wrappers for device types
static HomeAssistantView* g_homeAssistantView = nullptr;

static void selectLights() {
    if (g_homeAssistantView) {
        // Call through to controller
        DEBUG_PRINTLN("[HomeAssistantView] Lights selected");
    }
}

HomeAssistantView::HomeAssistantView()
    : m_controller(nullptr)
    , m_grid(nullptr)
    , m_slider(nullptr)
    , m_mode(HomeAssistantViewMode::DEVICE_TYPES)
    , m_selectedType(HomeAssistantDeviceType::LIGHT)
    , m_selectedDeviceIndex(-1)
    , m_devices(nullptr)
    , m_deviceCount(0)
    , m_isDragging(false)
    , m_showSlider(false)
    , m_lastUpdate(0) {
    
    m_isActive = false;
    m_lastTouch = {0, 0, false, 0};
}

HomeAssistantView::~HomeAssistantView() {
    if (m_grid) {
        delete m_grid;
    }
    if (m_slider) {
        delete m_slider;
    }
    if (m_devices) {
        delete[] m_devices;
    }
}

void HomeAssistantView::onEnter() {
    DEBUG_PRINTLN("[HomeAssistantView] Entering...");
    
    m_isActive = true;
    g_homeAssistantView = this;
    m_controller = &HomeAssistantController::getInstance();
    
    // Create hexagonal grid
    if (!m_grid) {
        m_grid = new HexagonalGrid(SCREEN_CENTER_X, SCREEN_CENTER_Y);
    }
    
    // Load device types
    m_mode = HomeAssistantViewMode::DEVICE_TYPES;
    loadDeviceTypes();
    
    DEBUG_PRINTLN("[HomeAssistantView] Entered");
}

void HomeAssistantView::onExit() {
    DEBUG_PRINTLN("[HomeAssistantView] Exiting...");
    m_isActive = false;
    g_homeAssistantView = nullptr;
}

void HomeAssistantView::update() {
    // Refresh devices every 10 seconds when in control view
    if (m_mode == HomeAssistantViewMode::DEVICE_CONTROL) {
        uint32_t currentTime = millis();
        if (currentTime - m_lastUpdate >= 10000) {
            // TODO: Refresh selected device state
            m_lastUpdate = currentTime;
        }
    }
}

void HomeAssistantView::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Clear background
    sprite->fillSprite(TFT_BLACK);

    // Draw circular border
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, TFT_ORANGE);
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 1, TFT_ORANGE);

    // Render based on mode
    switch (m_mode) {
        case HomeAssistantViewMode::DEVICE_TYPES:
            renderDeviceTypes();
            break;
        case HomeAssistantViewMode::DEVICE_LIST:
            renderDeviceList();
            break;
        case HomeAssistantViewMode::DEVICE_CONTROL:
            renderDeviceControl();
            break;
    }
}

void HomeAssistantView::renderDeviceTypes() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw title
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString("HOME ASSISTANT", SCREEN_CENTER_X, 15);

    // Draw hexagonal grid of device types
    if (m_grid) {
        m_grid->render();
    }

    // Draw instruction
    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->drawString("Select device type", SCREEN_CENTER_X, SCREEN_HEIGHT - 10);
}

void HomeAssistantView::renderDeviceList() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw title based on selected type
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    
    const char* typeTitle = "DEVICES";
    switch (m_selectedType) {
        case HomeAssistantDeviceType::LIGHT:
            typeTitle = "LIGHTS";
            break;
        case HomeAssistantDeviceType::CLIMATE:
            typeTitle = "CLIMATE";
            break;
        case HomeAssistantDeviceType::MEDIA_PLAYER:
            typeTitle = "MEDIA";
            break;
        case HomeAssistantDeviceType::SENSOR:
            typeTitle = "SENSORS";
            break;
        default:
            break;
    }
    sprite->drawString(typeTitle, SCREEN_CENTER_X, 15);

    // Draw device grid
    if (m_grid) {
        m_grid->render();
    }

    // Draw back hint
    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->drawString("Swipe down to go back", SCREEN_CENTER_X, SCREEN_HEIGHT - 10);
}

void HomeAssistantView::renderDeviceControl() {
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_deviceCount) {
        return;
    }

    const HomeAssistantDevice& device = m_devices[m_selectedDeviceIndex];

    // Render based on device type
    switch (device.type) {
        case HomeAssistantDeviceType::LIGHT:
            renderLightControl();
            break;
        case HomeAssistantDeviceType::CLIMATE:
            renderClimateControl();
            break;
        case HomeAssistantDeviceType::MEDIA_PLAYER:
            renderMediaPlayerControl();
            break;
        case HomeAssistantDeviceType::SENSOR:
            renderSensorDisplay();
            break;
        default:
            break;
    }
}

void HomeAssistantView::renderLightControl() {
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_deviceCount) return;
    
    const HomeAssistantDevice& device = m_devices[m_selectedDeviceIndex];
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw device name
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString(device.friendlyName.c_str(), SCREEN_CENTER_X, 20);

    // Draw light bulb icon
    int16_t iconY = SCREEN_CENTER_Y - 40;
    int16_t iconRadius = 50;
    
    // Color based on state
    uint16_t bulbColor = (device.state == HomeAssistantDeviceState::ON) ? 
                          TFT_YELLOW : TFT_DARKGREY;
    
    // Draw bulb
    sprite->fillCircle(SCREEN_CENTER_X, iconY, iconRadius, bulbColor);
    
    // Draw brightness if on
    if (device.state == HomeAssistantDeviceState::ON && device.brightness > 0) {
        sprite->setTextColor(TFT_BLACK);
        sprite->setTextDatum(MC_DATUM);
        sprite->setTextSize(2);
        char brightnessStr[5];
        snprintf(brightnessStr, sizeof(brightnessStr), "%d%%", (device.brightness * 100) / 255);
        sprite->drawString(brightnessStr, SCREEN_CENTER_X, iconY);
    }

    // State text
    sprite->setTextColor(TFT_LIGHTGREY);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(1);
    const char* stateText = (device.state == HomeAssistantDeviceState::ON) ? "ON" : "OFF";
    sprite->drawString(stateText, SCREEN_CENTER_X, iconY + iconRadius + 20);

    // Draw slider if shown
    if (m_showSlider && m_slider) {
        m_slider->render();
    }

    // Draw control hints
    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->drawString("Tap: ON/OFF • Long press: Brightness", SCREEN_CENTER_X, SCREEN_HEIGHT - 10);
}

void HomeAssistantView::renderClimateControl() {
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_deviceCount) return;
    
    const HomeAssistantDevice& device = m_devices[m_selectedDeviceIndex];
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw device name
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString(device.friendlyName.c_str(), SCREEN_CENTER_X, 20);

    // Draw thermometer icon
    int16_t iconY = SCREEN_CENTER_Y - 30;
    sprite->setTextColor(TFT_CYAN);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(4);
    sprite->drawString("T", SCREEN_CENTER_X, iconY);

    // Draw temperature
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(2);
    char tempStr[10];
    snprintf(tempStr, sizeof(tempStr), "%.1f°C", device.temperature);
    sprite->drawString(tempStr, SCREEN_CENTER_X, iconY + 50);

    // Draw target temperature
    sprite->setTextColor(TFT_ORANGE);
    sprite->setTextSize(1);
    snprintf(tempStr, sizeof(tempStr), "Target: %.1f°C", device.targetTemperature);
    sprite->drawString(tempStr, SCREEN_CENTER_X, iconY + 80);

    // Draw slider if shown
    if (m_showSlider && m_slider) {
        m_slider->render();
    }

    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->drawString("Tap: Adjust temperature", SCREEN_CENTER_X, SCREEN_HEIGHT - 10);
}

void HomeAssistantView::renderMediaPlayerControl() {
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_deviceCount) return;
    
    const HomeAssistantDevice& device = m_devices[m_selectedDeviceIndex];
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw device name
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString(device.friendlyName.c_str(), SCREEN_CENTER_X, 20);

    // Draw speaker icon
    int16_t iconY = SCREEN_CENTER_Y - 40;
    int16_t iconRadius = 50;
    sprite->fillCircle(SCREEN_CENTER_X, iconY, iconRadius, TFT_PURPLE);
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(3);
    sprite->drawString("♪", SCREEN_CENTER_X, iconY);

    // Draw media info
    if (device.mediaTitle.length() > 0) {
        sprite->setTextColor(TFT_LIGHTGREY);
        sprite->setTextSize(1);
        sprite->drawString(device.mediaTitle.c_str(), SCREEN_CENTER_X, iconY + iconRadius + 20);
        
        if (device.mediaArtist.length() > 0) {
            sprite->setTextColor(TFT_DARKGREY);
            sprite->drawString(device.mediaArtist.c_str(), SCREEN_CENTER_X, iconY + iconRadius + 40);
        }
    }

    // Draw slider for volume
    if (m_showSlider && m_slider) {
        m_slider->render();
    }

    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->drawString("Tap: Play/Pause • Long press: Volume", SCREEN_CENTER_X, SCREEN_HEIGHT - 10);
}

void HomeAssistantView::renderSensorDisplay() {
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_deviceCount) return;
    
    const HomeAssistantDevice& device = m_devices[m_selectedDeviceIndex];
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw device name
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString(device.friendlyName.c_str(), SCREEN_CENTER_X, 40);

    // Draw sensor value
    sprite->setTextColor(TFT_CYAN);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(3);
    sprite->drawString(device.sensorValue.c_str(), SCREEN_CENTER_X, SCREEN_CENTER_Y);

    // Draw unit
    if (device.unit.length() > 0) {
        sprite->setTextColor(TFT_LIGHTGREY);
        sprite->setTextSize(1);
        sprite->drawString(device.unit.c_str(), SCREEN_CENTER_X, SCREEN_CENTER_Y + 40);
    }

    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->drawString("Read-only sensor", SCREEN_CENTER_X, SCREEN_HEIGHT - 10);
}

void HomeAssistantView::handleTouch(TouchEvent event) {
    TouchController& touchCtrl = TouchController::getInstance();
    TouchPoint currentTouch = touchCtrl.getCurrentTouch();

    switch (m_mode) {
        case HomeAssistantViewMode::DEVICE_TYPES:
            if (event == TouchEvent::TAP && m_grid) {
                m_grid->handleTap(currentTouch.x, currentTouch.y);
            } else if (event == TouchEvent::DRAG_START) {
                m_isDragging = true;
            } else if (event == TouchEvent::DRAG_MOVE && m_isDragging && m_grid) {
                int16_t deltaX = currentTouch.x - m_lastTouch.x;
                int16_t deltaY = currentTouch.y - m_lastTouch.y;
                m_grid->handleDrag(deltaX, deltaY);
            } else if (event == TouchEvent::DRAG_END) {
                m_isDragging = false;
            }
            break;

        case HomeAssistantViewMode::DEVICE_LIST:
            if (event == TouchEvent::TAP && m_grid) {
                m_grid->handleTap(currentTouch.x, currentTouch.y);
            } else if (event == TouchEvent::SWIPE_DOWN) {
                // Back to device types
                m_mode = HomeAssistantViewMode::DEVICE_TYPES;
                loadDeviceTypes();
            }
            break;

        case HomeAssistantViewMode::DEVICE_CONTROL:
            if (event == TouchEvent::TAP) {
                toggleDevicePower();
            } else if (event == TouchEvent::LONG_PRESS) {
                // Show slider
                m_showSlider = !m_showSlider;
            } else if (event == TouchEvent::SWIPE_DOWN) {
                // Back to device list
                m_mode = HomeAssistantViewMode::DEVICE_LIST;
                loadDeviceList(m_selectedType);
                m_showSlider = false;
            }
            break;
    }

    m_lastTouch = currentTouch;
}

void HomeAssistantView::loadDeviceTypes() {
    if (!m_grid) return;

    m_grid->clear();

    // Create icons for each device type
    createDeviceTypeIcon(HomeAssistantDeviceType::LIGHT, "Lights");
    createDeviceTypeIcon(HomeAssistantDeviceType::SWITCH, "Switches");
    createDeviceTypeIcon(HomeAssistantDeviceType::SENSOR, "Sensors");
    createDeviceTypeIcon(HomeAssistantDeviceType::CLIMATE, "Climate");
    createDeviceTypeIcon(HomeAssistantDeviceType::MEDIA_PLAYER, "Media");
    createDeviceTypeIcon(HomeAssistantDeviceType::COVER, "Covers");
    createDeviceTypeIcon(HomeAssistantDeviceType::FAN, "Fans");
    createDeviceTypeIcon(HomeAssistantDeviceType::LOCK, "Locks");

    DEBUG_PRINTF("[HomeAssistantView] Loaded %d device types\n", m_grid->getItemCount());
}

void HomeAssistantView::loadDeviceList(HomeAssistantDeviceType type) {
    if (!m_grid || !m_controller) return;

    m_grid->clear();
    m_selectedType = type;

    // Load devices of this type
    const int MAX_DEVICES = 20;
    if (m_devices) {
        delete[] m_devices;
    }
    m_devices = new HomeAssistantDevice[MAX_DEVICES];
    m_deviceCount = m_controller->getDevicesByType(type, m_devices, MAX_DEVICES);

    // Create icon for each device
    for (int i = 0; i < m_deviceCount; i++) {
        createDeviceIcon(m_devices[i]);
    }

    DEBUG_PRINTF("[HomeAssistantView] Loaded %d devices\n", m_deviceCount);
}

void HomeAssistantView::createDeviceTypeIcon(HomeAssistantDeviceType type, const char* label) {
    GridItem item;
    item.label = label;
    item.icon = nullptr;
    item.userData = (void*)type;

    // Set color based on type
    switch (type) {
        case HomeAssistantDeviceType::LIGHT:
            item.backgroundColor = TFT_YELLOW;
            break;
        case HomeAssistantDeviceType::CLIMATE:
            item.backgroundColor = TFT_CYAN;
            break;
        case HomeAssistantDeviceType::MEDIA_PLAYER:
            item.backgroundColor = TFT_PURPLE;
            break;
        case HomeAssistantDeviceType::SENSOR:
            item.backgroundColor = TFT_GREEN;
            break;
        default:
            item.backgroundColor = TFT_DARKGREY;
    }

    item.onTap = [type]() {
        if (g_homeAssistantView) {
            g_homeAssistantView->selectDeviceType(type);
        }
    };

    m_grid->addItem(item);
}

void HomeAssistantView::createDeviceIcon(const HomeAssistantDevice& device) {
    GridItem item;
    item.label = device.friendlyName.c_str();
    item.icon = nullptr;
    
    // Color based on state
    if (device.state == HomeAssistantDeviceState::ON) {
        item.backgroundColor = TFT_GREEN;
    } else if (device.state == HomeAssistantDeviceState::OFF) {
        item.backgroundColor = TFT_DARKGREY;
    } else {
        item.backgroundColor = TFT_RED;
    }

    int deviceIndex = m_grid->getItemCount();
    item.userData = (void*)(intptr_t)deviceIndex;
    item.onTap = [deviceIndex]() {
        if (g_homeAssistantView) {
            g_homeAssistantView->selectDevice(deviceIndex);
        }
    };

    m_grid->addItem(item);
}

void HomeAssistantView::selectDeviceType(HomeAssistantDeviceType type) {
    DEBUG_PRINTF("[HomeAssistantView] Selected device type: %d\n", (int)type);
    m_mode = HomeAssistantViewMode::DEVICE_LIST;
    loadDeviceList(type);
}

void HomeAssistantView::selectDevice(int deviceIndex) {
    DEBUG_PRINTF("[HomeAssistantView] Selected device: %d\n", deviceIndex);
    m_selectedDeviceIndex = deviceIndex;
    m_mode = HomeAssistantViewMode::DEVICE_CONTROL;
    m_showSlider = false;
    
    // Create slider if needed
    if (!m_slider) {
        m_slider = new CircularSlider(SCREEN_CENTER_X, SCREEN_CENTER_Y, 100, 80);
    }
}

void HomeAssistantView::toggleDevicePower() {
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_deviceCount || !m_controller) {
        return;
    }

    HomeAssistantDevice& device = m_devices[m_selectedDeviceIndex];
    bool turnOn = (device.state == HomeAssistantDeviceState::OFF);

    DEBUG_PRINTF("[HomeAssistantView] Toggle device %s: %s\n", 
                 device.entityId.c_str(), turnOn ? "ON" : "OFF");

    m_controller->toggleDevice(device.entityId.c_str(), turnOn);
    
    // Update local state
    device.state = turnOn ? HomeAssistantDeviceState::ON : HomeAssistantDeviceState::OFF;
}

void HomeAssistantView::updateBrightness(float value) {
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_deviceCount) return;
    
    HomeAssistantDevice& device = m_devices[m_selectedDeviceIndex];
    device.brightness = (uint8_t)(value * 255);
    
    // TODO: Send to Home Assistant
}

void HomeAssistantView::updateHue(float value) {
    // TODO: Implement hue control
}

void HomeAssistantView::updateTemperature(float value) {
    // TODO: Implement temperature control
}

void HomeAssistantView::updateVolume(float value) {
    if (m_selectedDeviceIndex < 0 || m_selectedDeviceIndex >= m_deviceCount) return;
    
    HomeAssistantDevice& device = m_devices[m_selectedDeviceIndex];
    device.volume = value;
    
    // TODO: Send to Home Assistant
}

PageView* createHomeAssistantView() {
    return new HomeAssistantView();
}


