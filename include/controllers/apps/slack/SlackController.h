/**
 * @file SlackController.h
 * @brief Slack controller for API integration - MVC Controller Layer
 * 
 * Manages Slack API interactions and real-time notifications.
 * Part of MVC architecture - Controller layer (app controller).
 */

#ifndef SLACK_CONTROLLER_H
#define SLACK_CONTROLLER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "services/DatabaseService.h"
#include "services/AuthService.h"
#include "services/NetworkService.h"
#include "models/slack/SlackNotification.h"

/**
 * @enum SlackNotificationType
 * @brief Types of Slack notifications
 */
enum class SlackNotificationType {
    MESSAGE,
    MENTION,
    CHANNEL_UPDATE,
    DIRECT_MESSAGE,
    CALL,
    FILE_SHARED,
    UNKNOWN
};

/**
 * @class SlackController
 * @brief Singleton controller for Slack integration
 * 
 * Features:
 * - Real-time notifications
 * - Message monitoring
 * - Channel tracking
 * - Mention alerts
 * - Call notifications
 * - API authentication
 * - Token management
 */
class SlackController {
public:
    /**
     * @brief Get singleton instance
     */
    static SlackController& getInstance();

    /**
     * @brief Initialize controller
     * @return true if successful
     */
    bool init();

    /**
     * @brief Update controller (poll for new notifications)
     */
    void update();

    /**
     * @brief Set OAuth token
     * @param token Slack OAuth token
     */
    void setToken(const String& token);

    /**
     * @brief Get OAuth token
     * @return OAuth token
     */
    String getToken();

    /**
     * @brief Authenticate with Slack
     * @return true if authenticated
     */
    bool authenticate();

    /**
     * @brief Check if authenticated
     * @return true if authenticated
     */
    bool isAuthenticated();

    /**
     * @brief Get unread notification count
     * @return Number of unread notifications
     */
    int getUnreadCount();

    /**
     * @brief Get latest notification
     * @return Pointer to notification (or nullptr)
     */
    SlackNotification* getLatestNotification();

    /**
     * @brief Get all notifications
     * @param notifications Output array
     * @param maxNotifications Maximum notifications to return
     * @return Number of notifications
     */
    int getNotifications(SlackNotification* notifications, int maxNotifications);

    /**
     * @brief Clear all notifications
     */
    void clearNotifications();

    /**
     * @brief Mark notification as read
     * @param notificationId Notification ID
     */
    void markAsRead(const String& notificationId);

    /**
     * @brief Get workspace name
     * @return Workspace name
     */
    String getWorkspaceName();

    /**
     * @brief Get user info
     * @return User display name
     */
    String getUserDisplayName();

    /**
     * @brief Fetch conversations
     * @return true if successful
     */
    bool fetchConversations();

    /**
     * @brief Fetch messages from a channel
     * @param channelId Channel ID
     * @return true if successful
     */
    bool fetchMessages(const String& channelId);

    /**
     * @brief Send a message
     * @param channelId Channel ID
     * @param text Message text
     * @return true if successful
     */
    bool sendMessage(const String& channelId, const String& text);

private:
    SlackController();
    ~SlackController();
    SlackController(const SlackController&) = delete;
    SlackController& operator=(const SlackController&) = delete;

    bool makeAPIRequest(const String& endpoint, const String& method, const String& payload, String& response);
    void parseNotifications(const String& jsonResponse);
    void addNotification(const SlackNotification& notification);

    String m_token;
    String m_workspaceName;
    String m_userDisplayName;
    String m_userId;
    bool m_authenticated;
    bool m_initialized;

    SlackNotification* m_notifications;
    int m_notificationCount;
    int m_maxNotifications;
    int m_unreadCount;

    uint32_t m_lastPollTime;
    uint32_t m_pollInterval;  // Polling interval in ms
};

#endif // SLACK_CONTROLLER_H


