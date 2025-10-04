/**
 * @file NavigationController.cpp
 * @brief Implementation of NavigationController
 */

#include "controllers/NavigationController.h"
#include "services/AuthService.h"
#include "config/Config.h"

NavigationController& NavigationController::getInstance() {
    static NavigationController instance;
    return instance;
}

NavigationController::NavigationController()
    : m_currentRoute(nullptr)
    , m_rootRoute(nullptr) {
}

NavigationController::~NavigationController() {
    reset();
}

bool NavigationController::init() {
    DEBUG_PRINTLN("[NavigationController] Initializing...");

    // Register default routes
    registerDefaultRoutes();

    // Set root route
    m_rootRoute = findRoute("/");
    if (!m_rootRoute) {
        DEBUG_PRINTLN("[NavigationController] ERROR: No root route defined!");
        return false;
    }

    // Navigate to root
    bool success = navigateTo("/", true);

    DEBUG_PRINTLN("[NavigationController] Initialized successfully");
    return success;
}

void NavigationController::registerDefaultRoutes() {
    // TODO: Register default routes
    // These will be defined when page views are implemented
    
    // Example routes:
    // registerRoute({"/"         , "Home"        , createHomeView        , nullptr, false, nullptr});
    // registerRoute({"/lock"     , "Lock"        , createLockView        , nullptr, false, nullptr});
    // registerRoute({"/login"    , "Login"       , createLoginView       , nullptr, false, nullptr});
    // registerRoute({"/settings" , "Settings"    , createSettingsView    , nullptr, true , nullptr});
    // registerRoute({"/app/slack", "Slack"       , createSlackView       , nullptr, true , nullptr});
    // registerRoute({"/app/spotify", "Spotify"   , createSpotifyView     , nullptr, true , nullptr});

    DEBUG_PRINTLN("[NavigationController] TODO: Register actual page routes");
}

bool NavigationController::navigateTo(const char* path, bool clearStack) {
    if (!path) {
        DEBUG_PRINTLN("[NavigationController] ERROR: null path");
        return false;
    }

    DEBUG_PRINTF("[NavigationController] Navigating to: %s\n", path);

    // Find route
    Route* route = findRoute(path);
    if (!route) {
        DEBUG_PRINTF("[NavigationController] ERROR: Route not found: %s\n", path);
        return false;
    }

    // Check route guard
    if (!canActivateRoute(route)) {
        DEBUG_PRINTF("[NavigationController] Access denied: %s\n", path);
        
        // Redirect to guard's redirect path
        if (route->guard) {
            const char* redirectPath = route->guard->getRedirectPath();
            if (strcmp(redirectPath, path) != 0) {  // Prevent redirect loop
                return navigateTo(redirectPath, false);
            }
        }
        return false;
    }

    // Clear stack if requested
    if (clearStack) {
        while (!m_stack.empty()) {
            PageView* page = m_stack.back();
            m_stack.pop_back();
            cleanupPage(page);
        }
    }

    // Exit current page
    if (!m_stack.empty()) {
        m_stack.back()->onExit();
    }

    // Create new page view
    PageView* newPage = route->createView();
    if (!newPage) {
        DEBUG_PRINTF("[NavigationController] ERROR: Failed to create view for: %s\n", path);
        return false;
    }

    // Push to stack
    m_stack.push_back(newPage);
    m_currentRoute = route;

    // Enter new page
    newPage->onEnter();

    DEBUG_PRINTF("[NavigationController] Navigation successful. Stack depth: %d\n", m_stack.size());
    return true;
}

bool NavigationController::goBack() {
    if (!canGoBack()) {
        DEBUG_PRINTLN("[NavigationController] Cannot go back - at root");
        return false;
    }

    DEBUG_PRINTLN("[NavigationController] Going back...");

    // Exit current page
    PageView* currentPage = m_stack.back();
    currentPage->onExit();
    
    // Remove from stack
    m_stack.pop_back();
    cleanupPage(currentPage);

    // Enter previous page
    if (!m_stack.empty()) {
        PageView* previousPage = m_stack.back();
        previousPage->onEnter();
        
        // Update current route (search for matching route)
        // TODO: Store route reference in page or maintain route stack
        m_currentRoute = nullptr;  // Will need to track this better
    }

    DEBUG_PRINTF("[NavigationController] Back navigation successful. Stack depth: %d\n", m_stack.size());
    return true;
}

PageView* NavigationController::getCurrentPage() {
    if (m_stack.empty()) {
        return nullptr;
    }
    return m_stack.back();
}

Route* NavigationController::getCurrentRoute() {
    return m_currentRoute;
}

bool NavigationController::canGoBack() const {
    return m_stack.size() > 1;
}

void NavigationController::reset() {
    DEBUG_PRINTLN("[NavigationController] Resetting navigation stack...");

    // Clean up all pages
    for (PageView* page : m_stack) {
        cleanupPage(page);
    }
    m_stack.clear();
    
    m_currentRoute = nullptr;

    // Navigate to root if it exists
    if (m_rootRoute) {
        navigateTo("/", true);
    }
}

void NavigationController::registerRoute(const Route& route) {
    // Check if route already exists
    for (const auto& existing : m_routes) {
        if (strcmp(existing.path, route.path) == 0) {
            DEBUG_PRINTF("[NavigationController] WARNING: Route already registered: %s\n", route.path);
            return;
        }
    }

    m_routes.push_back(route);
    DEBUG_PRINTF("[NavigationController] Registered route: %s -> %s\n", route.path, route.name);
}

Route* NavigationController::findRoute(const char* path) {
    for (auto& route : m_routes) {
        if (strcmp(route.path, path) == 0) {
            return &route;
        }
    }
    return nullptr;
}

void NavigationController::update() {
    PageView* currentPage = getCurrentPage();
    if (currentPage) {
        currentPage->update();
    }
}

void NavigationController::render() {
    PageView* currentPage = getCurrentPage();
    if (currentPage) {
        currentPage->render();
    }
}

void NavigationController::handleTouch(TouchEvent event) {
    PageView* currentPage = getCurrentPage();
    if (currentPage) {
        currentPage->handleTouch(event);
    }
}

bool NavigationController::canActivateRoute(const Route* route) {
    if (!route) return false;

    // Check route guard
    if (route->guard) {
        return route->guard->canActivate(route);
    }

    // Check authentication requirement
    if (route->requiresAuth) {
        // TODO: Implement AuthService and uncomment
        // return AuthService::getInstance().isAuthenticated();
        DEBUG_PRINTLN("[NavigationController] TODO: Implement AuthService check");
        return true;  // Temporary - allow all for now
    }

    return true;
}

void NavigationController::cleanupPage(PageView* page) {
    if (page) {
        delete page;
    }
}

// LoginGuard implementation
bool LoginGuard::canActivate(const Route* route) {
    // TODO: Implement AuthService and uncomment
    // return AuthService::getInstance().isAuthenticated();
    
    DEBUG_PRINTLN("[LoginGuard] TODO: Implement authentication check");
    return true;  // Temporary - allow all for now
}


