/**
 * @file LockView.cpp
 * @brief Implementation of LockView
 */

#include "views/pages/LockView.h"
#include "controllers/NavigationController.h"
#include "hardware/display/DisplayDriver.h"
#include <time.h>

LockView::LockView()
    : m_currentTab(LockTab::CLOCK)
    , m_hours(0)
    , m_minutes(0)
    , m_seconds(0)
    , m_day(1)
    , m_month(1)
    , m_year(2024)
    , m_dayOfWeek("Monday")
    , m_lastTimeUpdate(0) {
    
    m_isActive = false;
}

LockView::~LockView() {
}

void LockView::onEnter() {
    DEBUG_PRINTLN("[LockView] Entering...");
    
    m_isActive = true;
    m_currentTab = LockTab::CLOCK;
    
    // Update time immediately
    updateTime();

    DEBUG_PRINTLN("[LockView] Entered");
}

void LockView::onExit() {
    DEBUG_PRINTLN("[LockView] Exiting...");
    m_isActive = false;
}

void LockView::update() {
    // Update time every second
    uint32_t currentTime = millis();
    if (currentTime - m_lastTimeUpdate >= 1000) {
        updateTime();
        m_lastTimeUpdate = currentTime;
    }
}

void LockView::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Clear background
    sprite->fillSprite(TFT_BLACK);

    // Draw circular border
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, TFT_BLUE);
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 1, TFT_BLUE);

    // Render current tab
    switch (m_currentTab) {
        case LockTab::CLOCK:
            renderClock();
            break;
        case LockTab::CALENDAR:
            renderCalendar();
            break;
        case LockTab::WEATHER:
            renderWeather();
            break;
    }

    // Render battery status on top
    renderBatteryStatus();

    // Draw unlock instruction
    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->drawString("Swipe up to unlock", SCREEN_CENTER_X, SCREEN_HEIGHT - 20);
}

void LockView::renderClock() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Format time string (HH:MM)
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", m_hours, m_minutes);

    // Draw large time
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(MC_DATUM);
    sprite->setFreeFont(&FreeSansBold24pt7b);  // Large font
    sprite->drawString(timeStr, SCREEN_CENTER_X, SCREEN_CENTER_Y - 20);

    // Draw seconds (smaller)
    char secStr[3];
    snprintf(secStr, sizeof(secStr), "%02d", m_seconds);
    sprite->setTextColor(TFT_DARKGREY);
    sprite->setFreeFont(&FreeSans12pt7b);
    sprite->drawString(secStr, SCREEN_CENTER_X, SCREEN_CENTER_Y + 30);

    // Draw date
    char dateStr[32];
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    snprintf(dateStr, sizeof(dateStr), "%s, %s %d, %d", 
             m_dayOfWeek.c_str(), months[m_month - 1], m_day, m_year);
    
    sprite->setTextColor(TFT_LIGHTGREY);
    sprite->drawString(dateStr, SCREEN_CENTER_X, SCREEN_CENTER_Y + 60);
}

void LockView::renderCalendar() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // TODO: Implement calendar view with events from Google Calendar
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(MC_DATUM);
    sprite->drawString("CALENDAR", SCREEN_CENTER_X, SCREEN_CENTER_Y - 40);
    
    sprite->setTextColor(TFT_DARKGREY);
    sprite->drawString("No events today", SCREEN_CENTER_X, SCREEN_CENTER_Y);
    
    sprite->setTextColor(TFT_LIGHTGREY);
    sprite->drawString("(Coming soon)", SCREEN_CENTER_X, SCREEN_CENTER_Y + 30);
}

void LockView::renderWeather() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // TODO: Implement weather view with data from weather API
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(MC_DATUM);
    sprite->drawString("WEATHER", SCREEN_CENTER_X, SCREEN_CENTER_Y - 40);
    
    // Placeholder weather data
    sprite->setFreeFont(&FreeSansBold24pt7b);
    sprite->drawString("22°C", SCREEN_CENTER_X, SCREEN_CENTER_Y);
    
    sprite->setFreeFont(&FreeSans12pt7b);
    sprite->setTextColor(TFT_LIGHTGREY);
    sprite->drawString("Partly Cloudy", SCREEN_CENTER_X, SCREEN_CENTER_Y + 40);
    
    sprite->setTextColor(TFT_DARKGREY);
    sprite->drawString("(Weather API needed)", SCREEN_CENTER_X, SCREEN_CENTER_Y + 70);
}

void LockView::renderBatteryStatus() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    BatteryMonitor& battery = BatteryMonitor::getInstance();
    uint8_t batteryLevel = battery.getBatteryLevel();
    bool isCharging = battery.isCharging();

    // Draw battery icon in top-right
    int16_t battX = SCREEN_WIDTH - 40;
    int16_t battY = 20;
    
    // Battery outline
    sprite->drawRect(battX, battY, 30, 15, TFT_WHITE);
    sprite->fillRect(battX + 30, battY + 5, 3, 5, TFT_WHITE);  // Terminal
    
    // Battery fill (based on level)
    uint16_t fillColor = TFT_GREEN;
    if (batteryLevel < 20) fillColor = TFT_RED;
    else if (batteryLevel < 50) fillColor = TFT_YELLOW;
    
    int16_t fillWidth = (batteryLevel * 26) / 100;
    sprite->fillRect(battX + 2, battY + 2, fillWidth, 11, fillColor);
    
    // Draw charging indicator
    if (isCharging) {
        sprite->drawString("+", battX + 35, battY + 7);
    }
    
    // Draw percentage
    char battStr[5];
    snprintf(battStr, sizeof(battStr), "%d%%", batteryLevel);
    sprite->setTextColor(TFT_LIGHTGREY);
    sprite->setTextDatum(TR_DATUM);
    sprite->setFreeFont(nullptr);  // Small font
    sprite->drawString(battStr, battX - 5, battY);
}

void LockView::updateTime() {
    // Get current time from RTC or system
    // TODO: Implement RTC driver and use it
    
    // For now, use millis() as a demo
    uint32_t totalSeconds = millis() / 1000;
    m_hours = (totalSeconds / 3600) % 24;
    m_minutes = (totalSeconds / 60) % 60;
    m_seconds = totalSeconds % 60;
    
    // TODO: Get actual date from RTC
    m_day = 4;
    m_month = 10;
    m_year = 2024;
    m_dayOfWeek = "Friday";
}

void LockView::handleTouch(TouchEvent event) {
    TouchController& touchCtrl = TouchController::getInstance();
    
    switch (event) {
        case TouchEvent::SWIPE_UP:
            // Swipe up to unlock
            DEBUG_PRINTLN("[LockView] Swipe up detected - unlocking");
            
            // Check if user is logged in
            if (AuthService::getInstance().isAuthenticated()) {
                // Navigate to home
                NavigationController::getInstance().navigateTo("/", true);
            } else {
                // Navigate to login
                NavigationController::getInstance().navigateTo("/login", true);
            }
            break;

        case TouchEvent::SWIPE_LEFT:
            // Switch to next tab
            if (m_currentTab == LockTab::CLOCK) {
                m_currentTab = LockTab::CALENDAR;
            } else if (m_currentTab == LockTab::CALENDAR) {
                m_currentTab = LockTab::WEATHER;
            }
            DEBUG_PRINTF("[LockView] Switched to tab: %d\n", (int)m_currentTab);
            break;

        case TouchEvent::SWIPE_RIGHT:
            // Switch to previous tab
            if (m_currentTab == LockTab::WEATHER) {
                m_currentTab = LockTab::CALENDAR;
            } else if (m_currentTab == LockTab::CALENDAR) {
                m_currentTab = LockTab::CLOCK;
            }
            DEBUG_PRINTF("[LockView] Switched to tab: %d\n", (int)m_currentTab);
            break;

        default:
            break;
    }
}

PageView* createLockView() {
    return new LockView();
}


