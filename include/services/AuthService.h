/**
 * @file AuthService.h
 * @brief Authentication service - MVC Service Layer
 * 
 * Manages user authentication, session management, and multi-user support.
 * Part of MVC architecture - Service layer.
 */

#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <Arduino.h>
#include "models/User.h"

/**
 * @class AuthService
 * @brief Singleton service for user authentication
 * 
 * Features:
 * - User login/logout
 * - Session management
 * - Multi-user support
 * - Current user tracking
 */
class AuthService {
public:
    /**
     * @brief Get singleton instance
     */
    static AuthService& getInstance();

    /**
     * @brief Initialize authentication service
     * @return true if successful
     */
    bool init();

    /**
     * @brief Login user by ID
     * @param userId User ID
     * @return true if successful
     */
    bool login(int userId);

    /**
     * @brief Logout current user
     */
    void logout();

    /**
     * @brief Get current logged-in user
     * @return Pointer to current user or nullptr
     */
    User* getCurrentUser();

    /**
     * @brief Check if user is authenticated
     * @return true if user is logged in
     */
    bool isAuthenticated() const;

    /**
     * @brief Get current user ID
     * @return User ID or -1 if not authenticated
     */
    int getCurrentUserId() const;

private:
    AuthService();
    ~AuthService();
    AuthService(const AuthService&) = delete;
    AuthService& operator=(const AuthService&) = delete;

    User* m_currentUser;
    bool m_isAuthenticated;
};

#endif // AUTH_SERVICE_H


