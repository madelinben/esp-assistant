/**
 * @file DatabaseService.cpp
 * @brief Implementation of DatabaseService
 */

#include "services/DatabaseService.h"
#include "hardware/storage/SDCardDriver.h"
#include "utils/CryptoUtils.h"
#include "config/Config.h"

DatabaseService& DatabaseService::getInstance() {
    static DatabaseService instance;
    return instance;
}

DatabaseService::DatabaseService()
    : m_db(nullptr)
    , m_initialized(false) {
}

DatabaseService::~DatabaseService() {
    close();
}

bool DatabaseService::init() {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[DatabaseService] Initializing...");

    // Ensure SD card is mounted
    if (!SDCardDriver::getInstance().isMounted()) {
        DEBUG_PRINTLN("[DatabaseService] SD card not mounted, attempting to mount...");
        if (!SDCardDriver::getInstance().init()) {
            DEBUG_PRINTLN("[DatabaseService] Failed to mount SD card");
            return false;
        }
    }

    // Create database directory if it doesn't exist
    if (!SDCardDriver::getInstance().exists("/database")) {
        if (!SDCardDriver::getInstance().createDirectory("/database")) {
            DEBUG_PRINTLN("[DatabaseService] Failed to create database directory");
            return false;
        }
    }

    // Open database
    int result = sqlite3_open(m_dbPath, &m_db);
    if (result != SQLITE_OK) {
        DEBUG_PRINTF("[DatabaseService] Failed to open database: %s\n", sqlite3_errmsg(m_db));
        return false;
    }

    DEBUG_PRINTLN("[DatabaseService] Database opened successfully");

    // Create tables
    if (!createTables()) {
        DEBUG_PRINTLN("[DatabaseService] Failed to create tables");
        close();
        return false;
    }

    m_initialized = true;
    DEBUG_PRINTLN("[DatabaseService] Initialized successfully");
    return true;
}

void DatabaseService::close() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
    m_initialized = false;
}

bool DatabaseService::createTables() {
    const char* createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            profile_image_path TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";

    const char* createAppConfigsTable = R"(
        CREATE TABLE IF NOT EXISTS app_configs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            app_name TEXT NOT NULL,
            enabled BOOLEAN DEFAULT 1,
            config_json TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id),
            UNIQUE(user_id, app_name)
        );
    )";

    const char* createTokensTable = R"(
        CREATE TABLE IF NOT EXISTS api_tokens (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            app_name TEXT NOT NULL,
            token_encrypted TEXT NOT NULL,
            token_type TEXT,
            expires_at TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id),
            UNIQUE(user_id, app_name)
        );
    )";

    const char* createSettingsTable = R"(
        CREATE TABLE IF NOT EXISTS settings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            setting_key TEXT NOT NULL,
            setting_value TEXT,
            FOREIGN KEY (user_id) REFERENCES users(id),
            UNIQUE(user_id, setting_key)
        );
    )";

    const char* createNotificationsTable = R"(
        CREATE TABLE IF NOT EXISTS notifications (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            app_name TEXT,
            title TEXT,
            message TEXT,
            timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            read BOOLEAN DEFAULT 0,
            FOREIGN KEY (user_id) REFERENCES users(id)
        );
    )";

    return executeSQL(createUsersTable) &&
           executeSQL(createAppConfigsTable) &&
           executeSQL(createTokensTable) &&
           executeSQL(createSettingsTable) &&
           executeSQL(createNotificationsTable);
}

bool DatabaseService::executeSQL(const String& sql) {
    char* errMsg = nullptr;
    int result = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (result != SQLITE_OK) {
        DEBUG_PRINTF("[DatabaseService] SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

// User operations
int DatabaseService::createUser(const User& user) {
    if (!m_initialized) return -1;

    String sql = "INSERT INTO users (username, profile_image_path) VALUES ('" +
                 user.getUsername() + "', '" + user.getProfileImagePath() + "');";
    
    if (executeSQL(sql)) {
        return sqlite3_last_insert_rowid(m_db);
    }
    
    return -1;
}

User* DatabaseService::getUserById(int id) {
    if (!m_initialized) return nullptr;

    String sql = "SELECT id, username, profile_image_path FROM users WHERE id = " + String(id) + ";";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return nullptr;
    }

    User* user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int userId = sqlite3_column_int(stmt, 0);
        String username = String((const char*)sqlite3_column_text(stmt, 1));
        String profilePath = String((const char*)sqlite3_column_text(stmt, 2));
        
        user = new User(userId, username, profilePath);
    }

    sqlite3_finalize(stmt);
    return user;
}

User** DatabaseService::getAllUsers(int& count) {
    if (!m_initialized) {
        count = 0;
        return nullptr;
    }

    String sql = "SELECT id, username, profile_image_path FROM users;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        count = 0;
        return nullptr;
    }

    // Count results first
    count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }
    sqlite3_reset(stmt);

    if (count == 0) {
        sqlite3_finalize(stmt);
        return nullptr;
    }

    // Allocate array
    User** users = new User*[count];
    int index = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int userId = sqlite3_column_int(stmt, 0);
        String username = String((const char*)sqlite3_column_text(stmt, 1));
        String profilePath = String((const char*)sqlite3_column_text(stmt, 2));
        
        users[index++] = new User(userId, username, profilePath);
    }

    sqlite3_finalize(stmt);
    return users;
}

bool DatabaseService::updateUser(const User& user) {
    if (!m_initialized) return false;

    String sql = "UPDATE users SET username = '" + user.getUsername() +
                 "', profile_image_path = '" + user.getProfileImagePath() +
                 "' WHERE id = " + String(user.getId()) + ";";
    
    return executeSQL(sql);
}

bool DatabaseService::deleteUser(int id) {
    if (!m_initialized) return false;

    String sql = "DELETE FROM users WHERE id = " + String(id) + ";";
    return executeSQL(sql);
}

// App configuration operations
bool DatabaseService::saveAppConfig(const AppConfig& config) {
    if (!m_initialized) return false;

    String sql = "INSERT OR REPLACE INTO app_configs (user_id, app_name, enabled, config_json) VALUES (" +
                 String(config.userId) + ", '" + config.appName + "', " +
                 String(config.enabled ? 1 : 0) + ", '" + config.configJson + "');";
    
    return executeSQL(sql);
}

AppConfig* DatabaseService::getAppConfig(int userId, const String& appName) {
    if (!m_initialized) return nullptr;

    String sql = "SELECT id, user_id, app_name, enabled, config_json FROM app_configs " +
                 String("WHERE user_id = ") + String(userId) + " AND app_name = '" + appName + "';";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return nullptr;
    }

    AppConfig* config = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        config = new AppConfig();
        config->id = sqlite3_column_int(stmt, 0);
        config->userId = sqlite3_column_int(stmt, 1);
        config->appName = String((const char*)sqlite3_column_text(stmt, 2));
        config->enabled = sqlite3_column_int(stmt, 3) == 1;
        config->configJson = String((const char*)sqlite3_column_text(stmt, 4));
    }

    sqlite3_finalize(stmt);
    return config;
}

AppConfig** DatabaseService::getUserAppConfigs(int userId, int& count) {
    if (!m_initialized) {
        count = 0;
        return nullptr;
    }

    String sql = "SELECT id, user_id, app_name, enabled, config_json FROM app_configs WHERE user_id = " +
                 String(userId) + " AND enabled = 1;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        count = 0;
        return nullptr;
    }

    // Count results
    count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count++;
    }
    sqlite3_reset(stmt);

    if (count == 0) {
        sqlite3_finalize(stmt);
        return nullptr;
    }

    // Allocate array
    AppConfig** configs = new AppConfig*[count];
    int index = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        AppConfig* config = new AppConfig();
        config->id = sqlite3_column_int(stmt, 0);
        config->userId = sqlite3_column_int(stmt, 1);
        config->appName = String((const char*)sqlite3_column_text(stmt, 2));
        config->enabled = sqlite3_column_int(stmt, 3) == 1;
        config->configJson = String((const char*)sqlite3_column_text(stmt, 4));
        
        configs[index++] = config;
    }

    sqlite3_finalize(stmt);
    return configs;
}

bool DatabaseService::deleteAppConfig(int userId, const String& appName) {
    if (!m_initialized) return false;

    String sql = "DELETE FROM app_configs WHERE user_id = " + String(userId) +
                 " AND app_name = '" + appName + "';";
    
    return executeSQL(sql);
}

// Token operations (encrypted)
bool DatabaseService::saveToken(int userId, const String& appName, const String& token, const String& tokenType) {
    if (!m_initialized) return false;

    String encryptedToken = encrypt(token);
    
    String sql = "INSERT OR REPLACE INTO api_tokens (user_id, app_name, token_encrypted, token_type) VALUES (" +
                 String(userId) + ", '" + appName + "', '" + encryptedToken + "', '" + tokenType + "');";
    
    return executeSQL(sql);
}

String DatabaseService::getToken(int userId, const String& appName) {
    if (!m_initialized) return "";

    String sql = "SELECT token_encrypted FROM api_tokens WHERE user_id = " + String(userId) +
                 " AND app_name = '" + appName + "';";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return "";
    }

    String token = "";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        String encryptedToken = String((const char*)sqlite3_column_text(stmt, 0));
        token = decrypt(encryptedToken);
    }

    sqlite3_finalize(stmt);
    return token;
}

bool DatabaseService::deleteToken(int userId, const String& appName) {
    if (!m_initialized) return false;

    String sql = "DELETE FROM api_tokens WHERE user_id = " + String(userId) +
                 " AND app_name = '" + appName + "';";
    
    return executeSQL(sql);
}

// Settings operations
bool DatabaseService::saveSetting(int userId, const String& key, const String& value) {
    if (!m_initialized) return false;

    String sql = "INSERT OR REPLACE INTO settings (user_id, setting_key, setting_value) VALUES (" +
                 String(userId) + ", '" + key + "', '" + value + "');";
    
    return executeSQL(sql);
}

String DatabaseService::getSetting(int userId, const String& key, const String& defaultValue) {
    if (!m_initialized) return defaultValue;

    String sql = "SELECT setting_value FROM settings WHERE user_id = " + String(userId) +
                 " AND setting_key = '" + key + "';";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return defaultValue;
    }

    String value = defaultValue;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        value = String((const char*)sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return value;
}

bool DatabaseService::executeQuery(const String& sql) {
    return executeSQL(sql);
}

// Encryption helpers
String DatabaseService::encrypt(const String& plaintext) {
    if (plaintext.length() == 0) {
        return "";
    }

    // Derive encryption key from master key
    uint8_t key[32];
    String salt = "esp_assistant_salt_v1";  // Static salt for simplicity
    if (!CryptoUtils::deriveKey(m_encryptionKey, salt, 10000, 32, key)) {
        DEBUG_PRINTLN("[DatabaseService] Failed to derive encryption key");
        return "";
    }

    // Encrypt plaintext
    String ciphertext;
    if (!CryptoUtils::encrypt(plaintext, key, ciphertext)) {
        DEBUG_PRINTLN("[DatabaseService] Failed to encrypt data");
        return "";
    }

    return ciphertext;
}

String DatabaseService::decrypt(const String& ciphertext) {
    if (ciphertext.length() == 0) {
        return "";
    }

    // Derive encryption key from master key
    uint8_t key[32];
    String salt = "esp_assistant_salt_v1";  // Static salt for simplicity
    if (!CryptoUtils::deriveKey(m_encryptionKey, salt, 10000, 32, key)) {
        DEBUG_PRINTLN("[DatabaseService] Failed to derive decryption key");
        return "";
    }

    // Decrypt ciphertext
    String plaintext;
    if (!CryptoUtils::decrypt(ciphertext, key, plaintext)) {
        DEBUG_PRINTLN("[DatabaseService] Failed to decrypt data");
        return "";
    }

    return plaintext;
}

