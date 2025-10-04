/**
 * @file main.cpp
 * @brief Main entry point for ESP32-S3 Touch LCD Assistant
 * 
 * ESP32-S3-Touch-LCD-1.85C Personal Assistant Device
 * Circular 360x360 touchscreen display with app integrations
 * 
 * @author Ben Madelin
 * @date 2024-10-04
 */

#include <Arduino.h>
#include "config/Config.h"

// Hardware drivers
#include "hardware/display/DisplayDriver.h"
#include "hardware/touch/TouchDriver.h"
#include "hardware/storage/SDCardDriver.h"
#include "hardware/power/BatteryMonitor.h"

// Controllers
#include "controllers/TouchController.h"
#include "controllers/NavigationController.h"

// Services
#include "services/AuthService.h"
#include "services/NetworkService.h"
#include "services/DatabaseService.h"

// App Controllers
#include "controllers/apps/spotify/SpotifyController.h"
#include "controllers/apps/slack/SlackController.h"
#include "controllers/apps/home-assistant/HomeAssistantController.h"

// Views - Pages
#include "views/pages/HomeView.h"
#include "views/pages/LockView.h"
#include "views/pages/LoginView.h"
#include "views/pages/SettingsView.h"
#include "views/pages/NotificationView.h"

// Views - Apps
#include "views/apps/slack/SlackView.h"
#include "views/apps/spotify/SpotifyView.h"
#include "views/apps/home-assistant/HomeAssistantView.h"

// Global system state
bool systemInitialized = false;
uint32_t lastFrameTime = 0;

/**
 * @brief Register all page routes
 */
void registerRoutes() {
    NavigationController& nav = NavigationController::getInstance();
    
    // Public routes (no authentication required)
    Route lockRoute = {
        .path = "/lock",
        .name = "Lock Screen",
        .createView = createLockView,
        .guard = nullptr,
        .requiresAuth = false,
        .userData = nullptr
    };
    nav.registerRoute(lockRoute);
    
    Route loginRoute = {
        .path = "/login",
        .name = "Login",
        .createView = createLoginView,
        .guard = nullptr,
        .requiresAuth = false,
        .userData = nullptr
    };
    nav.registerRoute(loginRoute);
    
    // Protected routes (authentication required)
    Route homeRoute = {
        .path = "/",
        .name = "Home",
        .createView = createHomeView,
        .guard = nullptr,  // TODO: Add LoginGuard
        .requiresAuth = true,
        .userData = nullptr
    };
    nav.registerRoute(homeRoute);
    
    Route settingsRoute = {
        .path = "/settings",
        .name = "Settings",
        .createView = createSettingsView,
        .guard = nullptr,
        .requiresAuth = true,
        .userData = nullptr
    };
    nav.registerRoute(settingsRoute);
    
    Route notificationRoute = {
        .path = "/notification",
        .name = "Notifications",
        .createView = createNotificationView,
        .guard = nullptr,
        .requiresAuth = true,
        .userData = nullptr
    };
    nav.registerRoute(notificationRoute);
    
    // App routes
    Route spotifyRoute = {
        .path = "/app/spotify",
        .name = "Spotify",
        .createView = createSpotifyView,
        .guard = nullptr,
        .requiresAuth = true,
        .userData = nullptr
    };
    nav.registerRoute(spotifyRoute);
    
    Route slackRoute = {
        .path = "/app/slack",
        .name = "Slack",
        .createView = createSlackView,
        .guard = nullptr,
        .requiresAuth = true,
        .userData = nullptr
    };
    nav.registerRoute(slackRoute);
    
    Route homeAssistantRoute = {
        .path = "/app/home-assistant",
        .name = "Home Assistant",
        .createView = createHomeAssistantView,
        .guard = nullptr,
        .requiresAuth = true,
        .userData = nullptr
    };
    nav.registerRoute(homeAssistantRoute);
    
    DEBUG_PRINTLN("[Main] All routes registered (Home, Lock, Login, Settings, Notification, Spotify, Slack, Home Assistant)");
}

/**
 * @brief Initialize all hardware subsystems
 * @return true if all systems initialized successfully
 */
bool initializeSystem() {
    DEBUG_PRINTLN("=================================");
    DEBUG_PRINTLN("ESP32-S3 Touch LCD Assistant V0");
    DEBUG_PRINTLN("=================================");
    DEBUG_PRINTLN();
    
    // Initialize Serial for debugging
    Serial.begin(SERIAL_BAUD);
    while (!Serial && millis() < 3000) {
        delay(10);
    }
    
    DEBUG_PRINTLN("Initializing hardware...");
    
    // Initialize Display
    if (!DisplayDriver::getInstance().init()) {
        DEBUG_PRINTLN("[X] Display Driver - FAILED");
        return false;
    }
    DEBUG_PRINTLN("[✓] Display Driver");
    
    // Initialize Touch
    if (!TouchController::getInstance().init()) {
        DEBUG_PRINTLN("[X] Touch Controller - FAILED");
        return false;
    }
    DEBUG_PRINTLN("[✓] Touch Controller");
    
    // Initialize SD Card
    if (!SDCardDriver::getInstance().init()) {
        DEBUG_PRINTLN("[!] SD Card Driver - FAILED (non-critical)");
        // Continue anyway - SD card is important but not critical for testing
    } else {
        DEBUG_PRINTLN("[✓] SD Card Driver");
    }
    
    // Initialize Battery Monitor
    if (!BatteryMonitor::getInstance().init()) {
        DEBUG_PRINTLN("[!] Battery Monitor - FAILED (non-critical)");
    } else {
        DEBUG_PRINTLN("[✓] Battery Monitor");
    }
    
    // TODO: Initialize Audio Manager
    DEBUG_PRINTLN("[ ] Audio Driver (not implemented)");
    
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Initializing services...");
    
    // Initialize Authentication Service
    if (!AuthService::getInstance().init()) {
        DEBUG_PRINTLN("[X] Auth Service - FAILED");
        return false;
    }
    DEBUG_PRINTLN("[✓] Auth Service");
    
    // Initialize Network Service
    if (!NetworkService::getInstance().init()) {
        DEBUG_PRINTLN("[!] Network Service - FAILED (non-critical)");
    } else {
        DEBUG_PRINTLN("[✓] Network Service");
    }
    
    // Initialize Database Service
    if (!DatabaseService::getInstance().init()) {
        DEBUG_PRINTLN("[!] Database Service - FAILED (non-critical)");
        DEBUG_PRINTLN("[!] Some features will not work without database");
    } else {
        DEBUG_PRINTLN("[✓] Database Service");
    }
    
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Initializing app controllers...");
    
    // Initialize Spotify Controller
    if (!SpotifyController::getInstance().init()) {
        DEBUG_PRINTLN("[!] Spotify Controller - FAILED (non-critical)");
    } else {
        DEBUG_PRINTLN("[✓] Spotify Controller");
    }
    
    // Initialize Slack Controller
    if (!SlackController::getInstance().init()) {
        DEBUG_PRINTLN("[!] Slack Controller - FAILED (non-critical)");
    } else {
        DEBUG_PRINTLN("[✓] Slack Controller");
    }
    
    // Initialize Home Assistant Controller
    if (!HomeAssistantController::getInstance().init()) {
        DEBUG_PRINTLN("[!] Home Assistant Controller - FAILED (non-critical)");
    } else {
        DEBUG_PRINTLN("[✓] Home Assistant Controller");
    }
    
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("Initializing navigation...");
    
    // Register routes
    registerRoutes();
    
    // Initialize Navigation Controller
    if (!NavigationController::getInstance().init()) {
        DEBUG_PRINTLN("[X] Navigation Controller - FAILED");
        return false;
    }
    DEBUG_PRINTLN("[✓] Navigation Controller");
    
    // Navigate to lock screen
    NavigationController::getInstance().navigateTo("/lock", true);
    
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("=================================");
    DEBUG_PRINTLN("System initialization complete!");
    DEBUG_PRINTLN("=================================");
    DEBUG_PRINTLN();
    
    return true;
}

/**
 * @brief Arduino setup function - called once at startup
 */
void setup() {
    // Initialize all subsystems
    systemInitialized = initializeSystem();
    
    if (!systemInitialized) {
        DEBUG_PRINTLN("ERROR: System initialization failed!");
        DEBUG_PRINTLN("System halted. Please reset device.");
        while (1) {
            delay(1000);
        }
    }
    
    // TODO: Load user preferences from database
    // TODO: Navigate to lock screen page
    
    DEBUG_PRINTLN("Setup complete. Starting main loop...");
    lastFrameTime = millis();
}

/**
 * @brief Arduino main loop - called repeatedly
 */
void loop() {
    uint32_t currentTime = millis();
    uint32_t deltaTime = currentTime - lastFrameTime;
    
    // Frame rate limiting (30 FPS)
    if (deltaTime < FRAME_TIME_MS) {
        delay(FRAME_TIME_MS - deltaTime);
        return;
    }
    
    lastFrameTime = currentTime;
    
    // Update touch input
    TouchController::getInstance().update();
    TouchEvent touchEvent = TouchController::getInstance().getLastEvent();
    
    // Update network service
    NetworkService::getInstance().update();
    
    // Update app controllers (for polling)
    SpotifyController::getInstance().update();
    SlackController::getInstance().update();
    HomeAssistantController::getInstance().update();
    
    // Update current page
    NavigationController& nav = NavigationController::getInstance();
    nav.update();
    
    // Handle touch events
    if (touchEvent != TouchEvent::NONE) {
        nav.handleTouch(touchEvent);
    }
    
    // Render frame
    DisplayDriver& display = DisplayDriver::getInstance();
    display.clear(TFT_BLACK);
    
    // Draw circular border
    display.drawCircularBorder(BORDER_COLOR, BORDER_WIDTH);
    
    // Render current page
    nav.render();
    
    // Swap buffers (display frame)
    display.swapBuffers();
    
    // Periodic status logging (every 5 seconds)
    static uint32_t lastStatusLog = 0;
    if (currentTime - lastStatusLog > 5000) {
        DEBUG_PRINTF("[Main] Free heap: %d bytes | Stack depth: %d | FPS: ~%d\n", 
                     ESP.getFreeHeap(), 
                     nav.getStackDepth(),
                     1000 / (deltaTime > 0 ? deltaTime : 1));
        lastStatusLog = currentTime;
    }
}

/**
 * @brief Optional: Setup function for dual-core usage
 * 
 * ESP32-S3 has two cores. You can use core 0 for UI rendering
 * and core 1 for network/API operations to improve performance.
 */
void setup1() {
    // TODO: Initialize network stack on core 1
    // TODO: Initialize API clients on core 1
}

void loop1() {
    // TODO: Handle network operations on core 1
    // TODO: Process API responses on core 1
    delay(10);
}