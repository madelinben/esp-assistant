/**
 * @file AuthService.cpp
 * @brief Implementation of AuthService
 */

#include "services/AuthService.h"
#include "services/DatabaseService.h"
#include "config/Config.h"

AuthService& AuthService::getInstance() {
    static AuthService instance;
    return instance;
}

AuthService::AuthService()
    : m_currentUser(nullptr)
    , m_isAuthenticated(false) {
}

AuthService::~AuthService() {
    logout();
}

bool AuthService::init() {
    DEBUG_PRINTLN("[AuthService] Initializing...");

    // Ensure database is initialized
    if (!DatabaseService::getInstance().init()) {
        DEBUG_PRINTLN("[AuthService] ERROR: Database not initialized");
        return false;
    }

    DEBUG_PRINTLN("[AuthService] Initialized successfully");
    return true;
}

bool AuthService::login(int userId) {
    DEBUG_PRINTF("[AuthService] Attempting login for user ID: %d\n", userId);

    // Logout current user if any
    if (m_isAuthenticated) {
        logout();
    }

    // Get user from database
    User* user = DatabaseService::getInstance().getUserById(userId);
    if (!user) {
        DEBUG_PRINTF("[AuthService] ERROR: User not found: %d\n", userId);
        return false;
    }

    if (!user->isValid()) {
        DEBUG_PRINTF("[AuthService] ERROR: Invalid user: %d\n", userId);
        delete user;
        return false;
    }

    // Set current user
    m_currentUser = user;
    m_currentUser->setActive(true);
    m_isAuthenticated = true;

    DEBUG_PRINTF("[AuthService] Login successful: %s (ID: %d)\n", 
                 m_currentUser->getUsername().c_str(), userId);
    
    return true;
}

void AuthService::logout() {
    if (!m_isAuthenticated) {
        return;
    }

    DEBUG_PRINTLN("[AuthService] Logging out...");

    if (m_currentUser) {
        m_currentUser->setActive(false);
        delete m_currentUser;
        m_currentUser = nullptr;
    }

    m_isAuthenticated = false;

    DEBUG_PRINTLN("[AuthService] Logout successful");
}

User* AuthService::getCurrentUser() {
    return m_currentUser;
}

bool AuthService::isAuthenticated() const {
    return m_isAuthenticated && m_currentUser != nullptr;
}

int AuthService::getCurrentUserId() const {
    if (m_isAuthenticated && m_currentUser) {
        return m_currentUser->getId();
    }
    return -1;
}


