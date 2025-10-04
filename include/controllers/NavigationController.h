/**
 * @file NavigationController.h
 * @brief Navigation controller with routing and guards - MVC Controller Layer
 * 
 * Manages page navigation using stack-based routing with support
 * for route guards (authentication) and state preservation.
 * Part of MVC architecture - Controller layer.
 */

#ifndef NAVIGATION_CONTROLLER_H
#define NAVIGATION_CONTROLLER_H

#include <Arduino.h>
#include <vector>
#include "controllers/TouchController.h"

// Forward declarations
class PageView;
class RouteGuard;

/**
 * @struct Route
 * @brief Route definition
 */
struct Route {
    const char* path;
    const char* name;
    PageView* (*createView)();  // Factory function
    RouteGuard* guard;
    bool requiresAuth;
    void* userData;
};

/**
 * @class NavigationController
 * @brief Singleton controller for page navigation
 * 
 * Features:
 * - Stack-based routing (push/pop)
 * - Route guards for authentication
 * - State preservation
 * - Back navigation support
 */
class NavigationController {
public:
    /**
     * @brief Get singleton instance
     */
    static NavigationController& getInstance();

    /**
     * @brief Initialize navigation system
     */
    bool init();

    /**
     * @brief Navigate to route by path
     * @param path Route path (e.g., "/lock", "/app/spotify")
     * @param clearStack If true, clear navigation stack first
     * @return true if navigation successful
     */
    bool navigateTo(const char* path, bool clearStack = false);

    /**
     * @brief Go back to previous page
     * @return true if successful (false if at root)
     */
    bool goBack();

    /**
     * @brief Get current page view
     * @return Pointer to current page or nullptr
     */
    PageView* getCurrentPage();

    /**
     * @brief Get current route
     * @return Pointer to current route or nullptr
     */
    Route* getCurrentRoute();

    /**
     * @brief Check if can navigate back
     * @return true if not at root page
     */
    bool canGoBack() const;

    /**
     * @brief Reset navigation to root page
     */
    void reset();

    /**
     * @brief Get navigation stack depth
     * @return Number of pages in stack
     */
    size_t getStackDepth() const { return m_stack.size(); }

    /**
     * @brief Register a route
     * @param route Route to register
     */
    void registerRoute(const Route& route);

    /**
     * @brief Find route by path
     * @param path Route path
     * @return Pointer to route or nullptr
     */
    Route* findRoute(const char* path);

    /**
     * @brief Update current page (call in main loop)
     */
    void update();

    /**
     * @brief Render current page
     */
    void render();

    /**
     * @brief Handle touch event
     * @param event Touch event
     */
    void handleTouch(TouchEvent event);

private:
    NavigationController();
    ~NavigationController();
    NavigationController(const NavigationController&) = delete;
    NavigationController& operator=(const NavigationController&) = delete;

    bool canActivateRoute(const Route* route);
    void cleanupPage(PageView* page);
    void registerDefaultRoutes();

    std::vector<PageView*> m_stack;
    std::vector<Route> m_routes;
    Route* m_currentRoute;
    Route* m_rootRoute;
};

/**
 * @class PageView
 * @brief Base class for all page views
 */
class PageView {
public:
    virtual ~PageView() {}

    /**
     * @brief Called when page is entered
     */
    virtual void onEnter() = 0;

    /**
     * @brief Called when page is exited
     */
    virtual void onExit() = 0;

    /**
     * @brief Update page logic (called each frame)
     */
    virtual void update() = 0;

    /**
     * @brief Render page content
     */
    virtual void render() = 0;

    /**
     * @brief Handle touch event
     * @param event Touch event type
     */
    virtual void handleTouch(TouchEvent event) = 0;

    /**
     * @brief Get page name
     */
    virtual const char* getName() const = 0;

protected:
    bool m_isActive;
};

/**
 * @class RouteGuard
 * @brief Base class for route guards (authentication, permissions)
 */
class RouteGuard {
public:
    virtual ~RouteGuard() {}

    /**
     * @brief Check if route can be activated
     * @param route Route to check
     * @return true if allowed, false to redirect
     */
    virtual bool canActivate(const Route* route) = 0;

    /**
     * @brief Get redirect path if guard fails
     * @return Path to redirect to
     */
    virtual const char* getRedirectPath() { return "/login"; }
};

/**
 * @class LoginGuard
 * @brief Route guard requiring user authentication
 */
class LoginGuard : public RouteGuard {
public:
    bool canActivate(const Route* route) override;
    const char* getRedirectPath() override { return "/login"; }
};

#endif // NAVIGATION_CONTROLLER_H


