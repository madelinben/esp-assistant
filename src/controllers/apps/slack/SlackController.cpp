/**
 * @file SlackController.cpp
 * @brief Implementation of SlackController
 */

#include "controllers/apps/slack/SlackController.h"

// Slack API constants
static const char* SLACK_API_BASE = "https://slack.com/api";
static const char* ENDPOINT_AUTH_TEST = "/auth.test";
static const char* ENDPOINT_CONVERSATIONS_LIST = "/conversations.list";
static const char* ENDPOINT_CONVERSATIONS_HISTORY = "/conversations.history";
static const char* ENDPOINT_USERS_INFO = "/users.info";
static const char* ENDPOINT_POST_MESSAGE = "/chat.postMessage";

SlackController& SlackController::getInstance() {
    static SlackController instance;
    return instance;
}

SlackController::SlackController()
    : m_token("")
    , m_workspaceName("")
    , m_userDisplayName("")
    , m_userId("")
    , m_authenticated(false)
    , m_initialized(false)
    , m_notifications(nullptr)
    , m_notificationCount(0)
    , m_maxNotifications(10)
    , m_unreadCount(0)
    , m_lastPollTime(0)
    , m_pollInterval(30000) {  // Poll every 30 seconds
    
    // Allocate notification buffer
    m_notifications = new SlackNotification[m_maxNotifications];
}

SlackController::~SlackController() {
    if (m_notifications) {
        delete[] m_notifications;
    }
}

bool SlackController::init() {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[SlackController] Initializing...");

    // Load token from database
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (currentUser) {
        String token = DatabaseService::getInstance().getToken(currentUser->getId(), "slack");
        if (token.length() > 0) {
            DEBUG_PRINTLN("[SlackController] Found saved token");
            setToken(token);
            authenticate();
        } else {
            DEBUG_PRINTLN("[SlackController] No saved token found");
        }
    }

    m_initialized = true;
    DEBUG_PRINTLN("[SlackController] Initialized");
    return true;
}

void SlackController::update() {
    if (!m_initialized || !m_authenticated) {
        return;
    }

    // Check if it's time to poll
    uint32_t currentTime = millis();
    if (currentTime - m_lastPollTime >= m_pollInterval) {
        m_lastPollTime = currentTime;
        
        // Fetch latest messages/notifications
        DEBUG_PRINTLN("[SlackController] Polling for updates...");
        fetchConversations();
    }
}

void SlackController::setToken(const String& token) {
    m_token = token;
    
    // Save token to database
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (currentUser) {
        DatabaseService::getInstance().saveToken(currentUser->getId(), "slack", token, "oauth");
    }
}

String SlackController::getToken() {
    return m_token;
}

bool SlackController::authenticate() {
    if (m_token.length() == 0) {
        DEBUG_PRINTLN("[SlackController] ERROR: No token set");
        return false;
    }

    if (!NetworkService::getInstance().isConnected()) {
        DEBUG_PRINTLN("[SlackController] ERROR: Not connected to network");
        return false;
    }

    DEBUG_PRINTLN("[SlackController] Authenticating...");

    String response;
    if (!makeAPIRequest(ENDPOINT_AUTH_TEST, "GET", "", response)) {
        DEBUG_PRINTLN("[SlackController] Authentication failed");
        m_authenticated = false;
        return false;
    }

    // Parse response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        DEBUG_PRINTF("[SlackController] JSON parse error: %s\n", error.c_str());
        m_authenticated = false;
        return false;
    }

    if (!doc["ok"].as<bool>()) {
        DEBUG_PRINTF("[SlackController] API error: %s\n", doc["error"].as<const char*>());
        m_authenticated = false;
        return false;
    }

    // Extract user info
    m_userId = doc["user_id"].as<String>();
    m_userDisplayName = doc["user"].as<String>();
    m_workspaceName = doc["team"].as<String>();

    m_authenticated = true;
    DEBUG_PRINTLN("[SlackController] Authenticated successfully!");
    DEBUG_PRINTF("[SlackController] User: %s, Workspace: %s\n", 
                 m_userDisplayName.c_str(), m_workspaceName.c_str());

    return true;
}

bool SlackController::isAuthenticated() {
    return m_authenticated;
}

int SlackController::getUnreadCount() {
    return m_unreadCount;
}

SlackNotification* SlackController::getLatestNotification() {
    if (m_notificationCount > 0) {
        return &m_notifications[0];
    }
    return nullptr;
}

int SlackController::getNotifications(SlackNotification* notifications, int maxNotifications) {
    if (!notifications || maxNotifications <= 0) {
        return 0;
    }

    int count = (m_notificationCount < maxNotifications) ? m_notificationCount : maxNotifications;
    
    for (int i = 0; i < count; i++) {
        notifications[i] = m_notifications[i];
    }

    return count;
}

void SlackController::clearNotifications() {
    m_notificationCount = 0;
    m_unreadCount = 0;
    DEBUG_PRINTLN("[SlackController] Notifications cleared");
}

void SlackController::markAsRead(const String& notificationId) {
    for (int i = 0; i < m_notificationCount; i++) {
        if (m_notifications[i].id == notificationId && !m_notifications[i].isRead) {
            m_notifications[i].isRead = true;
            m_unreadCount--;
            DEBUG_PRINTF("[SlackController] Marked notification as read: %s\n", notificationId.c_str());
            break;
        }
    }
}

String SlackController::getWorkspaceName() {
    return m_workspaceName;
}

String SlackController::getUserDisplayName() {
    return m_userDisplayName;
}

bool SlackController::fetchConversations() {
    if (!m_authenticated) {
        DEBUG_PRINTLN("[SlackController] Not authenticated");
        return false;
    }

    DEBUG_PRINTLN("[SlackController] Fetching conversations...");

    String response;
    if (!makeAPIRequest(ENDPOINT_CONVERSATIONS_LIST, "GET", "", response)) {
        DEBUG_PRINTLN("[SlackController] Failed to fetch conversations");
        return false;
    }

    // Parse and process conversations
    parseNotifications(response);

    return true;
}

bool SlackController::fetchMessages(const String& channelId) {
    if (!m_authenticated) {
        DEBUG_PRINTLN("[SlackController] Not authenticated");
        return false;
    }

    DEBUG_PRINTF("[SlackController] Fetching messages from channel: %s\n", channelId.c_str());

    String endpoint = String(ENDPOINT_CONVERSATIONS_HISTORY) + "?channel=" + channelId + "&limit=10";
    
    String response;
    if (!makeAPIRequest(endpoint, "GET", "", response)) {
        DEBUG_PRINTLN("[SlackController] Failed to fetch messages");
        return false;
    }

    // Parse messages and create notifications
    parseNotifications(response);

    return true;
}

bool SlackController::sendMessage(const String& channelId, const String& text) {
    if (!m_authenticated) {
        DEBUG_PRINTLN("[SlackController] Not authenticated");
        return false;
    }

    DEBUG_PRINTF("[SlackController] Sending message to channel: %s\n", channelId.c_str());

    // Create JSON payload
    DynamicJsonDocument doc(512);
    doc["channel"] = channelId;
    doc["text"] = text;

    String payload;
    serializeJson(doc, payload);

    String response;
    if (!makeAPIRequest(ENDPOINT_POST_MESSAGE, "POST", payload, response)) {
        DEBUG_PRINTLN("[SlackController] Failed to send message");
        return false;
    }

    DEBUG_PRINTLN("[SlackController] Message sent successfully");
    return true;
}

bool SlackController::makeAPIRequest(const String& endpoint, const String& method, const String& payload, String& response) {
    if (!NetworkService::getInstance().isConnected()) {
        DEBUG_PRINTLN("[SlackController] Not connected to network");
        return false;
    }

    HTTPClient http;
    String url = String(SLACK_API_BASE) + endpoint;

    DEBUG_PRINTF("[SlackController] %s %s\n", method.c_str(), url.c_str());

    http.begin(url);
    http.addHeader("Authorization", "Bearer " + m_token);
    http.addHeader("Content-Type", "application/json");

    int httpCode;
    if (method == "POST") {
        httpCode = http.POST(payload);
    } else {
        httpCode = http.GET();
    }

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            response = http.getString();
            http.end();
            return true;
        } else {
            DEBUG_PRINTF("[SlackController] HTTP error: %d\n", httpCode);
        }
    } else {
        DEBUG_PRINTF("[SlackController] Request failed: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    return false;
}

void SlackController::parseNotifications(const String& jsonResponse) {
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error) {
        DEBUG_PRINTF("[SlackController] JSON parse error: %s\n", error.c_str());
        return;
    }

    if (!doc["ok"].as<bool>()) {
        DEBUG_PRINTF("[SlackController] API error: %s\n", doc["error"].as<const char*>());
        return;
    }

    // Parse different response types (simplified example)
    // In a real implementation, you would parse specific fields based on the endpoint
    
    // Example: Parse messages
    JsonArray messages = doc["messages"];
    if (messages) {
        for (JsonObject msg : messages) {
            SlackNotification notification;
            notification.id = msg["ts"].as<String>();
            notification.text = msg["text"].as<String>();
            notification.channelId = msg["channel"].as<String>();
            notification.userId = msg["user"].as<String>();
            notification.timestamp = msg["ts"].as<String>();
            notification.isRead = false;
            notification.type = SlackNotificationType::MESSAGE;

            addNotification(notification);
        }
    }
}

void SlackController::addNotification(const SlackNotification& notification) {
    // Shift existing notifications down
    if (m_notificationCount >= m_maxNotifications) {
        for (int i = m_maxNotifications - 1; i > 0; i--) {
            m_notifications[i] = m_notifications[i - 1];
        }
        m_notificationCount = m_maxNotifications - 1;
    }

    // Add new notification at the top
    m_notifications[0] = notification;
    m_notificationCount++;
    m_unreadCount++;

    DEBUG_PRINTF("[SlackController] New notification: %s\n", notification.text.c_str());
}


