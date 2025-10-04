/**
 * @file Notification.h
 * @brief Notification model - MVC Model Layer
 * 
 * Represents a notification from any app.
 * Part of MVC architecture - Model layer.
 */

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <Arduino.h>

/**
 * @struct Notification
 * @brief Model representing a notification
 */
struct Notification {
    int id;
    int userId;
    String appName;        // App name (e.g., "Slack", "Spotify")
    String title;          // Notification title
    String message;        // Notification message
    String timestamp;      // Timestamp string for display
    uint32_t timestampMs;  // Timestamp in milliseconds
    uint16_t iconColor;    // Icon color (RGB565)
    bool isRead;           // Read status
    
    Notification()
        : id(0)
        , userId(0)
        , appName("")
        , title("")
        , message("")
        , timestamp("")
        , timestampMs(0)
        , iconColor(0xFFFF)
        , isRead(false) {}
    
    Notification(const String& app, const String& t, const String& msg, uint32_t ts)
        : id(0)
        , userId(0)
        , appName(app)
        , title(t)
        , message(msg)
        , timestamp("")
        , timestampMs(ts)
        , iconColor(0xFFFF)
        , isRead(false) {}
};

// Maximum number of notifications to keep in memory
#define MAX_NOTIFICATIONS 50

#endif // NOTIFICATION_H

