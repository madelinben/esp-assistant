/**
 * @file SettingsView.h
 * @brief Settings page view - MVC View Layer
 * 
 * Device settings page with hexagonal grid of setting categories.
 * Part of MVC architecture - View layer (page).
 */

#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include "controllers/NavigationController.h"
#include "views/layouts/HexagonalGrid.h"
#include "services/DatabaseService.h"
#include "services/AuthService.h"
#include "hardware/display/DisplayDriver.h"
#include "hardware/power/BatteryMonitor.h"

/**
 * @enum SettingsCategory
 * @brief Settings category types
 */
enum class SettingsCategory {
    DISPLAY,
    AUDIO,
    NETWORK,
    POWER,
    APPS,
    USER,
    ABOUT,
    LOGOUT
};

/**
 * @class SettingsView
 * @brief Settings page
 * 
 * Features:
 * - Hexagonal grid of setting categories
 * - Display settings (brightness)
 * - Audio settings (volume)
 * - Network settings (Wi-Fi)
 * - Power settings (sleep mode)
 * - App management
 * - User management
 * - Device information
 */
class SettingsView : public PageView {
public:
    /**
     * @brief Constructor
     */
    SettingsView();

    /**
     * @brief Destructor
     */
    ~SettingsView();

    // PageView interface
    void onEnter() override;
    void onExit() override;
    void update() override;
    void render() override;
    void handleTouch(TouchEvent event) override;
    const char* getName() const override { return "Settings"; }

private:
    void loadSettingsCategories();
    void createCategoryIcon(SettingsCategory category, const char* label);
    void showDisplaySettings();
    void showAudioSettings();
    void showNetworkSettings();
    void showPowerSettings();
    void showAbout();

    HexagonalGrid* m_grid;
    SettingsCategory m_selectedCategory;
    TouchPoint m_lastTouch;
    bool m_isDragging;
    bool m_inSubMenu;
};

/**
 * @brief Factory function for SettingsView
 * @return Pointer to new SettingsView instance
 */
PageView* createSettingsView();

#endif // SETTINGS_VIEW_H


