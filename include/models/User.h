/**
 * @file User.h
 * @brief User model - MVC Model Layer
 * 
 * Represents user data and authentication state.
 * Part of MVC architecture - Model layer.
 */

#ifndef USER_H
#define USER_H

#include <Arduino.h>

/**
 * @class User
 * @brief Model representing a user account
 */
class User {
public:
    User();
    User(int id, const String& username, const String& profileImagePath);

    // Getters
    int getId() const { return m_id; }
    String getUsername() const { return m_username; }
    String getProfileImagePath() const { return m_profileImagePath; }
    bool isActive() const { return m_isActive; }

    // Setters
    void setId(int id) { m_id = id; }
    void setUsername(const String& username) { m_username = username; }
    void setProfileImagePath(const String& path) { m_profileImagePath = path; }
    void setActive(bool active) { m_isActive = active; }

    // Validation
    bool isValid() const;

private:
    int m_id;
    String m_username;
    String m_profileImagePath;
    bool m_isActive;
};

#endif // USER_H


