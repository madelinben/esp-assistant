/**
 * @file NetworkService.cpp
 * @brief Implementation of NetworkService
 */

#include "services/NetworkService.h"
#include "services/DatabaseService.h"
#include "services/AuthService.h"

NetworkService& NetworkService::getInstance() {
    static NetworkService instance;
    return instance;
}

NetworkService::NetworkService()
    : m_status(NetworkStatus::DISCONNECTED)
    , m_ssid("")
    , m_password("")
    , m_autoReconnect(true)
    , m_lastReconnectAttempt(0)
    , m_reconnectInterval(30000)  // 30 seconds
    , m_initialized(false) {
}

NetworkService::~NetworkService() {
    disconnect();
}

bool NetworkService::init() {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[NetworkService] Initializing...");

    // Set Wi-Fi mode
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(m_autoReconnect);

    // Load saved credentials
    String savedSSID, savedPassword;
    if (loadCredentials(savedSSID, savedPassword)) {
        DEBUG_PRINTF("[NetworkService] Found saved credentials for: %s\n", savedSSID.c_str());
        m_ssid = savedSSID;
        m_password = savedPassword;
        
        // Auto-connect if credentials exist
        if (m_autoReconnect) {
            DEBUG_PRINTLN("[NetworkService] Auto-connecting...");
            connect(m_ssid, m_password);
        }
    } else {
        DEBUG_PRINTLN("[NetworkService] No saved credentials found");
    }

    m_initialized = true;
    DEBUG_PRINTLN("[NetworkService] Initialized");
    return true;
}

bool NetworkService::connect(const String& ssid, const String& password, uint32_t timeout) {
    if (ssid.length() == 0) {
        DEBUG_PRINTLN("[NetworkService] ERROR: Empty SSID");
        m_status = NetworkStatus::NO_SSID;
        return false;
    }

    DEBUG_PRINTF("[NetworkService] Connecting to: %s\n", ssid.c_str());
    m_status = NetworkStatus::CONNECTING;
    m_ssid = ssid;
    m_password = password;

    // Disconnect if already connected
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        delay(100);
    }

    // Start connection
    WiFi.begin(ssid.c_str(), password.c_str());

    // Wait for connection
    uint32_t startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeout) {
        delay(100);
        DEBUG_PRINT(".");
    }
    DEBUG_PRINTLN();

    if (WiFi.status() == WL_CONNECTED) {
        m_status = NetworkStatus::CONNECTED;
        DEBUG_PRINTLN("[NetworkService] Connected successfully!");
        DEBUG_PRINTF("[NetworkService] IP Address: %s\n", WiFi.localIP().toString().c_str());
        DEBUG_PRINTF("[NetworkService] Signal: %d dBm\n", WiFi.RSSI());
        
        // Save credentials
        saveCredentials(ssid, password);
        
        return true;
    } else {
        m_status = NetworkStatus::FAILED;
        DEBUG_PRINTLN("[NetworkService] Connection failed!");
        return false;
    }
}

void NetworkService::disconnect() {
    DEBUG_PRINTLN("[NetworkService] Disconnecting...");
    WiFi.disconnect();
    m_status = NetworkStatus::DISCONNECTED;
}

bool NetworkService::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

NetworkStatus NetworkService::getStatus() {
    // Update status based on WiFi status
    if (WiFi.status() == WL_CONNECTED) {
        m_status = NetworkStatus::CONNECTED;
    } else if (WiFi.status() == WL_CONNECT_FAILED) {
        m_status = NetworkStatus::FAILED;
    } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
        m_status = NetworkStatus::NO_SSID;
    } else if (WiFi.status() == WL_DISCONNECTED) {
        m_status = NetworkStatus::DISCONNECTED;
    }
    
    return m_status;
}

String NetworkService::getSSID() {
    if (isConnected()) {
        return WiFi.SSID();
    }
    return m_ssid;
}

String NetworkService::getIPAddress() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

int32_t NetworkService::getSignalStrength() {
    if (isConnected()) {
        return WiFi.RSSI();
    }
    return -100;  // Very weak/no signal
}

uint8_t NetworkService::getSignalQuality() {
    int32_t rssi = getSignalStrength();
    
    // Convert RSSI to percentage
    // -30 dBm = 100% (excellent)
    // -90 dBm = 0% (unusable)
    if (rssi >= -30) return 100;
    if (rssi <= -90) return 0;
    
    return (uint8_t)(((rssi + 90) * 100) / 60);
}

int NetworkService::scanNetworks(WiFiNetwork* networks, int maxNetworks) {
    if (!networks || maxNetworks <= 0) return 0;

    DEBUG_PRINTLN("[NetworkService] Scanning networks...");

    int numNetworks = WiFi.scanNetworks();
    
    if (numNetworks <= 0) {
        DEBUG_PRINTLN("[NetworkService] No networks found");
        return 0;
    }

    DEBUG_PRINTF("[NetworkService] Found %d networks\n", numNetworks);

    int count = 0;
    for (int i = 0; i < numNetworks && count < maxNetworks; i++) {
        networks[count].ssid = WiFi.SSID(i);
        networks[count].rssi = WiFi.RSSI(i);
        networks[count].encryptionType = WiFi.encryptionType(i);
        networks[count].channel = WiFi.channel(i);
        
        DEBUG_PRINTF("[NetworkService]   %d: %s (%d dBm)\n", 
                     i + 1, networks[count].ssid.c_str(), networks[count].rssi);
        count++;
    }

    // Clean up
    WiFi.scanDelete();

    return count;
}

bool NetworkService::startScan() {
    DEBUG_PRINTLN("[NetworkService] Starting async scan...");
    int result = WiFi.scanNetworks(true);  // Async scan
    return result != WIFI_SCAN_FAILED;
}

bool NetworkService::isScanComplete() {
    return WiFi.scanComplete() >= 0;
}

int NetworkService::getScanResults(WiFiNetwork* networks, int maxNetworks) {
    if (!networks || maxNetworks <= 0) return 0;

    int numNetworks = WiFi.scanComplete();
    
    if (numNetworks < 0) {
        return 0;  // Scan in progress or failed
    }

    int count = 0;
    for (int i = 0; i < numNetworks && count < maxNetworks; i++) {
        networks[count].ssid = WiFi.SSID(i);
        networks[count].rssi = WiFi.RSSI(i);
        networks[count].encryptionType = WiFi.encryptionType(i);
        networks[count].channel = WiFi.channel(i);
        count++;
    }

    return count;
}

void NetworkService::setAutoReconnect(bool enable) {
    m_autoReconnect = enable;
    WiFi.setAutoReconnect(enable);
    DEBUG_PRINTF("[NetworkService] Auto-reconnect: %s\n", enable ? "ON" : "OFF");
}

void NetworkService::update() {
    if (!m_initialized) return;

    // Handle auto-reconnect
    if (m_autoReconnect) {
        handleAutoReconnect();
    }

    // Update status
    getStatus();
}

void NetworkService::handleAutoReconnect() {
    // Only try to reconnect if disconnected and credentials exist
    if (isConnected() || m_ssid.length() == 0) {
        return;
    }

    uint32_t currentTime = millis();
    if (currentTime - m_lastReconnectAttempt >= m_reconnectInterval) {
        m_lastReconnectAttempt = currentTime;
        
        DEBUG_PRINTLN("[NetworkService] Auto-reconnect attempt...");
        connect(m_ssid, m_password, 10000);  // 10 second timeout
    }
}

void NetworkService::saveCredentials(const String& ssid, const String& password) {
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (!currentUser) {
        DEBUG_PRINTLN("[NetworkService] No user logged in, cannot save credentials");
        return;
    }

    // Save SSID as setting
    DatabaseService::getInstance().saveSetting(currentUser->getId(), "wifi_ssid", ssid);
    
    // Save password as encrypted token
    DatabaseService::getInstance().saveToken(currentUser->getId(), "wifi", password, "password");

    DEBUG_PRINTLN("[NetworkService] Credentials saved");
}

bool NetworkService::loadCredentials(String& ssid, String& password) {
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (!currentUser) {
        DEBUG_PRINTLN("[NetworkService] No user logged in, cannot load credentials");
        return false;
    }

    // Load SSID from settings
    ssid = DatabaseService::getInstance().getSetting(currentUser->getId(), "wifi_ssid", "");
    
    if (ssid.length() == 0) {
        return false;
    }

    // Load password from tokens
    password = DatabaseService::getInstance().getToken(currentUser->getId(), "wifi");

    return password.length() > 0;
}


