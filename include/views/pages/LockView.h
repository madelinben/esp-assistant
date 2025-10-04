/**
 * @file LockView.h
 * @brief Lock screen view with clock - MVC View Layer
 * 
 * Lock screen displaying time, date, weather, and calendar.
 * Part of MVC architecture - View layer (page).
 */

#ifndef LOCK_VIEW_H
#define LOCK_VIEW_H

#include "controllers/NavigationController.h"
#include "services/AuthService.h"
#include "hardware/power/BatteryMonitor.h"

/**
 * @enum LockTab
 * @brief Lock screen tabs
 */
enum class LockTab {
    CLOCK,
    CALENDAR,
    WEATHER
};

/**
 * @class LockView
 * @brief Lock screen page
 * 
 * Features:
 * - Large clock display (digital)
 * - Current date
 * - Battery status
 * - Swipe to unlock
 * - Tabs: Clock, Calendar, Weather
 */
class LockView : public PageView {
public:
    /**
     * @brief Constructor
     */
    LockView();

    /**
     * @brief Destructor
     */
    ~LockView();

    // PageView interface
    void onEnter() override;
    void onExit() override;
    void update() override;
    void render() override;
    void handleTouch(TouchEvent event) override;
    const char* getName() const override { return "Lock"; }

private:
    void renderClock();
    void renderCalendar();
    void renderWeather();
    void renderBatteryStatus();
    void updateTime();
    
    LockTab m_currentTab;
    
    // Time data
    int m_hours;
    int m_minutes;
    int m_seconds;
    int m_day;
    int m_month;
    int m_year;
    String m_dayOfWeek;
    
    uint32_t m_lastTimeUpdate;
};

/**
 * @brief Factory function for LockView
 * @return Pointer to new LockView instance
 */
PageView* createLockView();

#endif // LOCK_VIEW_H


