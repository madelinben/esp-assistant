/**
 * @file SettingsView.cpp
 * @brief Implementation of SettingsView
 */

#include "views/pages/SettingsView.h"
#include "controllers/NavigationController.h"

// Global callback wrappers
static SettingsView* g_settingsView = nullptr;

static void onDisplaySettings() {
    if (g_settingsView) {
        DEBUG_PRINTLN("[SettingsView] Display settings tapped");
        // TODO: Show display settings submenu
    }
}

static void onAudioSettings() {
    if (g_settingsView) {
        DEBUG_PRINTLN("[SettingsView] Audio settings tapped");
        // TODO: Show audio settings submenu
    }
}

static void onNetworkSettings() {
    if (g_settingsView) {
        DEBUG_PRINTLN("[SettingsView] Network settings tapped");
        // TODO: Show network settings submenu
    }
}

static void onPowerSettings() {
    if (g_settingsView) {
        DEBUG_PRINTLN("[SettingsView] Power settings tapped");
        // TODO: Show power settings submenu
    }
}

static void onAppsSettings() {
    if (g_settingsView) {
        DEBUG_PRINTLN("[SettingsView] Apps settings tapped");
        // TODO: Show apps management submenu
    }
}

static void onUserSettings() {
    if (g_settingsView) {
        DEBUG_PRINTLN("[SettingsView] User settings tapped");
        // TODO: Show user management submenu
    }
}

static void onAbout() {
    if (g_settingsView) {
        DEBUG_PRINTLN("[SettingsView] About tapped");
        // TODO: Show about/device info
    }
}

static void onLogout() {
    if (g_settingsView) {
        DEBUG_PRINTLN("[SettingsView] Logout tapped");
        AuthService::getInstance().logout();
        NavigationController::getInstance().navigateTo("/login", true);
    }
}

SettingsView::SettingsView()
    : m_grid(nullptr)
    , m_selectedCategory(SettingsCategory::DISPLAY)
    , m_isDragging(false)
    , m_inSubMenu(false) {
    
    m_isActive = false;
    m_lastTouch = {0, 0, false, 0};
}

SettingsView::~SettingsView() {
    if (m_grid) {
        delete m_grid;
    }
}

void SettingsView::onEnter() {
    DEBUG_PRINTLN("[SettingsView] Entering...");
    
    m_isActive = true;
    m_inSubMenu = false;
    g_settingsView = this;

    // Create hexagonal grid
    if (!m_grid) {
        m_grid = new HexagonalGrid(SCREEN_CENTER_X, SCREEN_CENTER_Y);
    }

    // Load settings categories
    loadSettingsCategories();

    DEBUG_PRINTLN("[SettingsView] Entered");
}

void SettingsView::onExit() {
    DEBUG_PRINTLN("[SettingsView] Exiting...");
    m_isActive = false;
    g_settingsView = nullptr;
}

void SettingsView::update() {
    // Update logic if needed
}

void SettingsView::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Clear background
    sprite->fillSprite(TFT_BLACK);

    // Draw circular border
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, TFT_DARKGREY);

    if (m_inSubMenu) {
        // Render submenu
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextDatum(MC_DATUM);
        sprite->drawString("Settings Submenu", SCREEN_CENTER_X, SCREEN_CENTER_Y);
        sprite->setTextColor(TFT_DARKGREY);
        sprite->drawString("(Coming soon)", SCREEN_CENTER_X, SCREEN_CENTER_Y + 30);
    } else {
        // Draw title
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextDatum(TC_DATUM);
        sprite->drawString("SETTINGS", SCREEN_CENTER_X, 20);

        // Draw hexagonal grid
        if (m_grid) {
            m_grid->render();
        }

        // Draw current user info at bottom
        User* currentUser = AuthService::getInstance().getCurrentUser();
        if (currentUser) {
            sprite->setTextColor(TFT_LIGHTGREY);
            sprite->setTextDatum(BC_DATUM);
            sprite->drawString(currentUser->getUsername(), SCREEN_CENTER_X, SCREEN_HEIGHT - 20);
        }
    }
}

void SettingsView::handleTouch(TouchEvent event) {
    TouchController& touchCtrl = TouchController::getInstance();
    TouchPoint currentTouch = touchCtrl.getCurrentTouch();

    if (m_inSubMenu) {
        // Handle submenu touches
        if (event == TouchEvent::SWIPE_DOWN || event == TouchEvent::TAP) {
            m_inSubMenu = false;
        }
    } else {
        // Handle main menu touches
        switch (event) {
            case TouchEvent::TAP:
                if (m_grid) {
                    m_grid->handleTap(currentTouch.x, currentTouch.y);
                }
                break;

            case TouchEvent::DRAG_START:
                m_isDragging = true;
                break;

            case TouchEvent::DRAG_MOVE:
                if (m_isDragging && m_grid) {
                    int16_t deltaX = currentTouch.x - m_lastTouch.x;
                    int16_t deltaY = currentTouch.y - m_lastTouch.y;
                    m_grid->handleDrag(deltaX, deltaY);
                }
                break;

            case TouchEvent::DRAG_END:
                m_isDragging = false;
                break;

            default:
                break;
        }
    }

    m_lastTouch = currentTouch;
}

void SettingsView::loadSettingsCategories() {
    if (!m_grid) return;

    DEBUG_PRINTLN("[SettingsView] Loading settings categories...");

    // Clear existing items
    m_grid->clear();

    // Add setting categories
    createCategoryIcon(SettingsCategory::DISPLAY, "Display");
    createCategoryIcon(SettingsCategory::AUDIO, "Audio");
    createCategoryIcon(SettingsCategory::NETWORK, "Network");
    createCategoryIcon(SettingsCategory::POWER, "Power");
    createCategoryIcon(SettingsCategory::APPS, "Apps");
    createCategoryIcon(SettingsCategory::USER, "User");
    createCategoryIcon(SettingsCategory::ABOUT, "About");
    createCategoryIcon(SettingsCategory::LOGOUT, "Logout");

    DEBUG_PRINTF("[SettingsView] Loaded %d categories\n", m_grid->getItemCount());
}

void SettingsView::createCategoryIcon(SettingsCategory category, const char* label) {
    GridItem item;
    item.label = label;
    item.icon = nullptr;
    item.userData = (void*)category;

    // Set color based on category
    switch (category) {
        case SettingsCategory::DISPLAY:
            item.backgroundColor = TFT_BLUE;
            item.onTap = onDisplaySettings;
            break;
        case SettingsCategory::AUDIO:
            item.backgroundColor = TFT_PURPLE;
            item.onTap = onAudioSettings;
            break;
        case SettingsCategory::NETWORK:
            item.backgroundColor = TFT_GREEN;
            item.onTap = onNetworkSettings;
            break;
        case SettingsCategory::POWER:
            item.backgroundColor = TFT_YELLOW;
            item.onTap = onPowerSettings;
            break;
        case SettingsCategory::APPS:
            item.backgroundColor = TFT_CYAN;
            item.onTap = onAppsSettings;
            break;
        case SettingsCategory::USER:
            item.backgroundColor = TFT_MAGENTA;
            item.onTap = onUserSettings;
            break;
        case SettingsCategory::ABOUT:
            item.backgroundColor = TFT_LIGHTGREY;
            item.onTap = onAbout;
            break;
        case SettingsCategory::LOGOUT:
            item.backgroundColor = TFT_RED;
            item.onTap = onLogout;
            break;
    }

    m_grid->addItem(item);
}

void SettingsView::showDisplaySettings() {
    m_inSubMenu = true;
    m_selectedCategory = SettingsCategory::DISPLAY;
    // TODO: Implement display settings submenu
}

void SettingsView::showAudioSettings() {
    m_inSubMenu = true;
    m_selectedCategory = SettingsCategory::AUDIO;
    // TODO: Implement audio settings submenu
}

void SettingsView::showNetworkSettings() {
    m_inSubMenu = true;
    m_selectedCategory = SettingsCategory::NETWORK;
    // TODO: Implement network settings submenu
}

void SettingsView::showPowerSettings() {
    m_inSubMenu = true;
    m_selectedCategory = SettingsCategory::POWER;
    // TODO: Implement power settings submenu
}

void SettingsView::showAbout() {
    m_inSubMenu = true;
    m_selectedCategory = SettingsCategory::ABOUT;
    // TODO: Show device information
}

PageView* createSettingsView() {
    return new SettingsView();
}


