/**
 * @file SlackNotification.h
 * @brief Slack notification data model - MVC Model Layer
 * 
 * Data structure for Slack notifications.
 * Part of MVC architecture - Model layer.
 */

#ifndef SLACK_NOTIFICATION_H
#define SLACK_NOTIFICATION_H

#include <Arduino.h>

// Forward declaration
enum class SlackNotificationType;

/**
 * @struct SlackNotification
 * @brief Slack notification data
 */
struct SlackNotification {
    String id;
    String text;
    String channelId;
    String channelName;
    String userId;
    String userName;
    String timestamp;
    String iconUrl;
    SlackNotificationType type;
    bool isRead;
    bool isMention;

    SlackNotification()
        : id("")
        , text("")
        , channelId("")
        , channelName("")
        , userId("")
        , userName("")
        , timestamp("")
        , iconUrl("")
        , isRead(false)
        , isMention(false) {}
};

#endif // SLACK_NOTIFICATION_H


