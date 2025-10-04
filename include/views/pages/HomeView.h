/**
 * @file HomeView.h
 * @brief Home page view with hexagonal app grid - MVC View Layer
 * 
 * Main menu page displaying apps in Apple Watch-style hexagonal grid.
 * Part of MVC architecture - View layer (page).
 */

#ifndef HOME_VIEW_H
#define HOME_VIEW_H

#include "controllers/NavigationController.h"
#include "views/layouts/HexagonalGrid.h"
#include "services/DatabaseService.h"
#include "services/AuthService.h"

/**
 * @class HomeView
 * @brief Main menu page with app grid
 * 
 * Features:
 * - Hexagonal grid of app icons
 * - Center-first spiral arrangement
 * - Tap to launch app
 * - Drag to scroll
 * - Dynamic app loading based on user config
 */
class HomeView : public PageView {
public:
    /**
     * @brief Constructor
     */
    HomeView();

    /**
     * @brief Destructor
     */
    ~HomeView();

    // PageView interface
    void onEnter() override;
    void onExit() override;
    void update() override;
    void render() override;
    void handleTouch(TouchEvent event) override;
    const char* getName() const override { return "Home"; }

private:
    void loadApps();
    void createAppIcon(const char* appName, const char* label);
    static void onAppTapped(const char* appPath);

    HexagonalGrid* m_grid;
    TouchPoint m_lastTouch;
    TouchPoint m_dragStart;
    bool m_isDragging;
};

/**
 * @brief Factory function for HomeView
 * @return Pointer to new HomeView instance
 */
PageView* createHomeView();

#endif // HOME_VIEW_H


