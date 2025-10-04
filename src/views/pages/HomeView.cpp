/**
 * @file HomeView.cpp
 * @brief Implementation of HomeView
 */

#include "views/pages/HomeView.h"
#include "controllers/NavigationController.h"
#include "hardware/display/DisplayDriver.h"

// Global callback wrappers (needed for C function pointers)
static NavigationController* g_navController = nullptr;

static void launchSlack() {
    if (g_navController) g_navController->navigateTo("/app/slack");
}

static void launchSpotify() {
    if (g_navController) g_navController->navigateTo("/app/spotify");
}

static void launchHomeAssistant() {
    if (g_navController) g_navController->navigateTo("/app/home-assistant");
}

static void launchAIAssistant() {
    if (g_navController) g_navController->navigateTo("/app/ai-assistant");
}

static void launchSettings() {
    if (g_navController) g_navController->navigateTo("/settings");
}

HomeView::HomeView()
    : m_grid(nullptr)
    , m_isDragging(false) {
    
    m_isActive = false;
    m_lastTouch = {0, 0, false, 0};
    m_dragStart = {0, 0, false, 0};
}

HomeView::~HomeView() {
    if (m_grid) {
        delete m_grid;
    }
}

void HomeView::onEnter() {
    DEBUG_PRINTLN("[HomeView] Entering...");
    
    m_isActive = true;
    g_navController = &NavigationController::getInstance();

    // Create hexagonal grid
    if (!m_grid) {
        m_grid = new HexagonalGrid(SCREEN_CENTER_X, SCREEN_CENTER_Y);
    }

    // Load apps for current user
    loadApps();

    DEBUG_PRINTLN("[HomeView] Entered");
}

void HomeView::onExit() {
    DEBUG_PRINTLN("[HomeView] Exiting...");
    m_isActive = false;
}

void HomeView::update() {
    // Update logic if needed
}

void HomeView::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Clear background
    sprite->fillSprite(TFT_BLACK);

    // Draw circular border
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, BORDER_COLOR);
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 1, BORDER_COLOR);

    // Draw hexagonal grid
    if (m_grid) {
        m_grid->render();
    }

    // Draw notification bar on border
    // TODO: Draw time, user name, notification badges on curved border
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->drawString("MAIN MENU", SCREEN_CENTER_X, 10);
}

void HomeView::handleTouch(TouchEvent event) {
    TouchController& touchCtrl = TouchController::getInstance();
    TouchPoint currentTouch = touchCtrl.getCurrentTouch();

    switch (event) {
        case TouchEvent::TAP:
            if (m_grid) {
                m_grid->handleTap(currentTouch.x, currentTouch.y);
            }
            break;

        case TouchEvent::DRAG_START:
            m_dragStart = currentTouch;
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

    m_lastTouch = currentTouch;
}

void HomeView::loadApps() {
    if (!m_grid) return;

    DEBUG_PRINTLN("[HomeView] Loading apps...");

    // Clear existing apps
    m_grid->clear();

    // Get current user
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (!currentUser) {
        DEBUG_PRINTLN("[HomeView] No user logged in, showing default apps");
        // Show default apps (Settings, Login)
        createAppIcon("settings", "Settings");
        return;
    }

    // Get user's app configurations from database
    int configCount = 0;
    AppConfig** configs = DatabaseService::getInstance().getUserAppConfigs(
        currentUser->getId(), configCount
    );

    if (configCount == 0) {
        DEBUG_PRINTLN("[HomeView] No app configs found, showing defaults");
        // Show default enabled apps
        createAppIcon("slack", "Slack");
        createAppIcon("spotify", "Spotify");
        createAppIcon("home-assistant", "Home");
        createAppIcon("settings", "Settings");
    } else {
        // Load apps from user configuration
        for (int i = 0; i < configCount; i++) {
            if (configs[i]->enabled) {
                createAppIcon(configs[i]->appName.c_str(), configs[i]->appName.c_str());
            }
            delete configs[i];
        }
        delete[] configs;
    }

    // Always show settings
    createAppIcon("settings", "Settings");

    DEBUG_PRINTF("[HomeView] Loaded %d apps\n", m_grid->getItemCount());
}

void HomeView::createAppIcon(const char* appName, const char* label) {
    GridItem item;
    item.label = label;
    item.icon = nullptr;  // TODO: Load actual icon
    item.backgroundColor = TFT_DARKGREY;
    item.userData = (void*)appName;

    // Set callback based on app name
    if (strcmp(appName, "slack") == 0) {
        item.onTap = launchSlack;
    } else if (strcmp(appName, "spotify") == 0) {
        item.onTap = launchSpotify;
    } else if (strcmp(appName, "home-assistant") == 0) {
        item.onTap = launchHomeAssistant;
    } else if (strcmp(appName, "ai-assistant") == 0) {
        item.onTap = launchAIAssistant;
    } else if (strcmp(appName, "settings") == 0) {
        item.onTap = launchSettings;
    }

    m_grid->addItem(item);
}

PageView* createHomeView() {
    return new HomeView();
}


