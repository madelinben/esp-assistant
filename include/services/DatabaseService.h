/**
 * @file DatabaseService.h
 * @brief SQLite database service with encryption - MVC Service Layer
 * 
 * Manages local SQLite database on SD card with AES-256 encryption
 * for sensitive data (API tokens, user data).
 * Part of MVC architecture - Service layer.
 */

#ifndef DATABASE_SERVICE_H
#define DATABASE_SERVICE_H

#include <Arduino.h>
#include <sqlite3.h>
#include "models/User.h"

/**
 * @struct AppConfig
 * @brief App configuration data
 */
struct AppConfig {
    int id;
    int userId;
    String appName;
    bool enabled;
    String configJson;  // Encrypted JSON configuration
};

/**
 * @class DatabaseService
 * @brief Singleton service for database operations
 * 
 * Handles all database operations including user management,
 * app configurations, and encrypted token storage.
 */
class DatabaseService {
public:
    /**
     * @brief Get singleton instance
     */
    static DatabaseService& getInstance();

    /**
     * @brief Initialize database and create tables
     * @return true if successful
     */
    bool init();

    /**
     * @brief Close database connection
     */
    void close();

    // User operations
    /**
     * @brief Create a new user
     * @param user User object
     * @return user ID or -1 on error
     */
    int createUser(const User& user);

    /**
     * @brief Get user by ID
     * @param id User ID
     * @return User object or nullptr
     */
    User* getUserById(int id);

    /**
     * @brief Get all users
     * @return Array of User pointers
     */
    User** getAllUsers(int& count);

    /**
     * @brief Update user
     * @param user User object
     * @return true if successful
     */
    bool updateUser(const User& user);

    /**
     * @brief Delete user
     * @param id User ID
     * @return true if successful
     */
    bool deleteUser(int id);

    // App configuration operations
    /**
     * @brief Save app configuration
     * @param config AppConfig object
     * @return true if successful
     */
    bool saveAppConfig(const AppConfig& config);

    /**
     * @brief Get app configuration
     * @param userId User ID
     * @param appName App name
     * @return AppConfig pointer or nullptr
     */
    AppConfig* getAppConfig(int userId, const String& appName);

    /**
     * @brief Get all app configs for user
     * @param userId User ID
     * @return Array of AppConfig pointers
     */
    AppConfig** getUserAppConfigs(int userId, int& count);

    /**
     * @brief Delete app configuration
     * @param userId User ID
     * @param appName App name
     * @return true if successful
     */
    bool deleteAppConfig(int userId, const String& appName);

    // Token operations (encrypted)
    /**
     * @brief Save encrypted API token
     * @param userId User ID
     * @param appName App name
     * @param token Token to encrypt and save
     * @param tokenType Token type (e.g., "bearer", "oauth")
     * @return true if successful
     */
    bool saveToken(int userId, const String& appName, const String& token, const String& tokenType);

    /**
     * @brief Get decrypted API token
     * @param userId User ID
     * @param appName App name
     * @return Decrypted token or empty string
     */
    String getToken(int userId, const String& appName);

    /**
     * @brief Delete token
     * @param userId User ID
     * @param appName App name
     * @return true if successful
     */
    bool deleteToken(int userId, const String& appName);

    // Settings operations
    /**
     * @brief Save setting
     * @param userId User ID
     * @param key Setting key
     * @param value Setting value
     * @return true if successful
     */
    bool saveSetting(int userId, const String& key, const String& value);

    /**
     * @brief Get setting
     * @param userId User ID
     * @param key Setting key
     * @param defaultValue Default value if not found
     * @return Setting value
     */
    String getSetting(int userId, const String& key, const String& defaultValue = "");

    /**
     * @brief Execute raw SQL query
     * @param sql SQL query
     * @return true if successful
     */
    bool executeQuery(const String& sql);

private:
    DatabaseService();
    ~DatabaseService();
    DatabaseService(const DatabaseService&) = delete;
    DatabaseService& operator=(const DatabaseService&) = delete;

    bool createTables();
    bool executeSQL(const String& sql);
    
    // Encryption helpers
    String encrypt(const String& plaintext);
    String decrypt(const String& ciphertext);

    sqlite3* m_db;
    bool m_initialized;
    const char* m_dbPath = "/sd/database/assistant.db";
    const char* m_encryptionKey = "CHANGE_ME_SECURE_KEY_32_BYTES!";  // TODO: Use secure key storage
};

#endif // DATABASE_SERVICE_H

