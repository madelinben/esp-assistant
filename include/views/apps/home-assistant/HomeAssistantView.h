/**
 * @file HomeAssistantView.h
 * @brief Home Assistant device control view - MVC View Layer
 * 
 * Smart home device control with hexagonal grid navigation.
 * Part of MVC architecture - View layer (app view).
 */

#ifndef HOME_ASSISTANT_VIEW_H
#define HOME_ASSISTANT_VIEW_H

#include <Arduino.h>
#include "controllers/NavigationController.h"
#include "controllers/TouchController.h"
#include "controllers/apps/home-assistant/HomeAssistantController.h"
#include "models/home-assistant/HomeAssistantDevice.h"
#include "views/layouts/HexagonalGrid.h"
#include "views/components/CircularSlider.h"
#include "services/AuthService.h"
#include "config/Config.h"

/**
 * @enum HomeAssistantViewMode
 * @brief View modes for navigation
 */
enum class HomeAssistantViewMode {
    DEVICE_TYPES,    // Grid of device types (lights, thermostats, etc.)
    DEVICE_LIST,     // Grid of devices of selected type
    DEVICE_CONTROL   // Control page for selected device
};

/**
 * @class HomeAssistantView
 * @brief Home Assistant device control view
 * 
 * Features:
 * - Device type grid (lights, thermostats, speakers, etc.)
 * - Device list grid (specific devices of type)
 * - Device control page (on/off, sliders, settings)
 * - Circular sliders for brightness, hue, volume, temperature
 * - Hexagonal grid navigation
 */
class HomeAssistantView : public PageView {
public:
    HomeAssistantView();
    ~HomeAssistantView() override;

    // PageView interface
    void onEnter() override;
    void onExit() override;
    void update() override;
    void render() override;
    void handleTouch(TouchEvent event) override;
    const char* getName() const override { return "Home Assistant"; }

private:
    // Rendering functions
    void renderDeviceTypes();
    void renderDeviceList();
    void renderDeviceControl();
    
    // Device control rendering
    void renderLightControl();
    void renderClimateControl();
    void renderMediaPlayerControl();
    void renderSensorDisplay();
    
    // Grid management
    void loadDeviceTypes();
    void loadDeviceList(HomeAssistantDeviceType type);
    void createDeviceTypeIcon(HomeAssistantDeviceType type, const char* label);
    void createDeviceIcon(const HomeAssistantDevice& device);
    
    // Device control
    void selectDeviceType(HomeAssistantDeviceType type);
    void selectDevice(int deviceIndex);
    void toggleDevicePower();
    void updateBrightness(float value);
    void updateHue(float value);
    void updateTemperature(float value);
    void updateVolume(float value);
    
    HomeAssistantController* m_controller;
    HexagonalGrid* m_grid;
    CircularSlider* m_slider;
    
    HomeAssistantViewMode m_mode;
    HomeAssistantDeviceType m_selectedType;
    int m_selectedDeviceIndex;
    HomeAssistantDevice* m_devices;
    int m_deviceCount;
    
    bool m_isDragging;
    bool m_showSlider;
    uint32_t m_lastUpdate;
    
    TouchPoint m_lastTouch;
};

// Factory function for navigation
PageView* createHomeAssistantView();

#endif // HOME_ASSISTANT_VIEW_H


