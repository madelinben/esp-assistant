/**
 * @file User.cpp
 * @brief Implementation of User model
 */

#include "models/User.h"

User::User()
    : m_id(-1)
    , m_username("")
    , m_profileImagePath("")
    , m_isActive(false) {
}

User::User(int id, const String& username, const String& profileImagePath)
    : m_id(id)
    , m_username(username)
    , m_profileImagePath(profileImagePath)
    , m_isActive(true) {
}

bool User::isValid() const {
    return m_id >= 0 && m_username.length() > 0;
}


