/**
 * @file NetworkService.h
 * @brief Network service for Wi-Fi management - MVC Service Layer
 * 
 * Manages Wi-Fi connections, network status, and connectivity.
 * Part of MVC architecture - Service layer.
 */

#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include "config/Config.h"

/**
 * @enum NetworkStatus
 * @brief Network connection status
 */
enum class NetworkStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    FAILED,
    NO_SSID
};

/**
 * @struct WiFiNetwork
 * @brief Wi-Fi network information
 */
struct WiFiNetwork {
    String ssid;
    int32_t rssi;
    uint8_t encryptionType;
    uint8_t channel;
};

/**
 * @class NetworkService
 * @brief Singleton service for network management
 * 
 * Features:
 * - Wi-Fi connection management
 * - Network scanning
 * - Auto-reconnect
 * - Signal strength monitoring
 * - Credential storage
 */
class NetworkService {
public:
    /**
     * @brief Get singleton instance
     */
    static NetworkService& getInstance();

    /**
     * @brief Initialize network service
     * @return true if successful
     */
    bool init();

    /**
     * @brief Connect to Wi-Fi network
     * @param ssid Network SSID
     * @param password Network password
     * @param timeout Connection timeout in milliseconds
     * @return true if connected
     */
    bool connect(const String& ssid, const String& password, uint32_t timeout = WIFI_TIMEOUT_MS);

    /**
     * @brief Disconnect from Wi-Fi
     */
    void disconnect();

    /**
     * @brief Check if connected
     * @return true if connected
     */
    bool isConnected();

    /**
     * @brief Get connection status
     * @return NetworkStatus enum
     */
    NetworkStatus getStatus();

    /**
     * @brief Get current SSID
     * @return SSID string
     */
    String getSSID();

    /**
     * @brief Get IP address
     * @return IP address string
     */
    String getIPAddress();

    /**
     * @brief Get signal strength (RSSI)
     * @return RSSI in dBm
     */
    int32_t getSignalStrength();

    /**
     * @brief Get signal quality percentage
     * @return Quality (0-100%)
     */
    uint8_t getSignalQuality();

    /**
     * @brief Scan for available networks
     * @param networks Output array of networks
     * @param maxNetworks Maximum networks to return
     * @return Number of networks found
     */
    int scanNetworks(WiFiNetwork* networks, int maxNetworks);

    /**
     * @brief Start async network scan
     * @return true if scan started
     */
    bool startScan();

    /**
     * @brief Check if scan is complete
     * @return true if complete
     */
    bool isScanComplete();

    /**
     * @brief Get scan results
     * @param networks Output array of networks
     * @param maxNetworks Maximum networks to return
     * @return Number of networks found
     */
    int getScanResults(WiFiNetwork* networks, int maxNetworks);

    /**
     * @brief Enable auto-reconnect
     * @param enable true to enable
     */
    void setAutoReconnect(bool enable);

    /**
     * @brief Update connection status (call periodically)
     */
    void update();

    /**
     * @brief Save Wi-Fi credentials
     * @param ssid Network SSID
     * @param password Network password
     */
    void saveCredentials(const String& ssid, const String& password);

    /**
     * @brief Load saved credentials
     * @param ssid Output SSID
     * @param password Output password
     * @return true if credentials found
     */
    bool loadCredentials(String& ssid, String& password);

private:
    NetworkService();
    ~NetworkService();
    NetworkService(const NetworkService&) = delete;
    NetworkService& operator=(const NetworkService&) = delete;

    void handleAutoReconnect();

    NetworkStatus m_status;
    String m_ssid;
    String m_password;
    bool m_autoReconnect;
    uint32_t m_lastReconnectAttempt;
    uint32_t m_reconnectInterval;
    bool m_initialized;
};

#endif // NETWORK_SERVICE_H


