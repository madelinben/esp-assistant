/**
 * @file NotificationView.cpp
 * @brief Implementation of NotificationView
 */

#include "views/pages/NotificationView.h"
#include "hardware/display/DisplayDriver.h"

NotificationView::NotificationView()
    : m_notifications(nullptr)
    , m_notificationCount(0)
    , m_currentIndex(0)
    , m_scrollOffset(0) {
    
    m_isActive = false;
    m_lastTouch = {0, 0, false, 0};
}

NotificationView::~NotificationView() {
    if (m_notifications) {
        delete[] m_notifications;
    }
}

void NotificationView::onEnter() {
    DEBUG_PRINTLN("[NotificationView] Entering...");
    
    m_isActive = true;
    loadNotifications();
    
    DEBUG_PRINTF("[NotificationView] Loaded %d notifications\n", m_notificationCount);
}

void NotificationView::onExit() {
    DEBUG_PRINTLN("[NotificationView] Exiting...");
    m_isActive = false;
}

void NotificationView::update() {
    // Could refresh notifications periodically
}

void NotificationView::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Clear background
    sprite->fillSprite(TFT_BLACK);

    // Draw circular border
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, TFT_CYAN);
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 1, TFT_CYAN);

    // Draw title
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString("NOTIFICATIONS", SCREEN_CENTER_X, 15);

    // Draw notification count
    if (m_notificationCount > 0) {
        int16_t badgeX = SCREEN_WIDTH - 30;
        int16_t badgeY = 25;
        sprite->fillCircle(badgeX, badgeY, 15, TFT_RED);
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextDatum(MC_DATUM);
        
        char countStr[4];
        snprintf(countStr, sizeof(countStr), "%d", m_notificationCount);
        sprite->drawString(countStr, badgeX, badgeY);
    }

    // Render notifications or empty state
    if (m_notificationCount > 0) {
        renderNotifications();
    } else {
        renderEmptyState();
    }

    // Draw controls
    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    if (m_notificationCount > 0) {
        sprite->drawString("Swipe: Navigate • Tap: Read • Long: Clear all", 
                          SCREEN_CENTER_X, SCREEN_HEIGHT - 5);
    } else {
        sprite->drawString("Swipe down to go back", SCREEN_CENTER_X, SCREEN_HEIGHT - 5);
    }
}

void NotificationView::renderNotifications() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite || m_notificationCount == 0) return;

    // Render current notification (centered)
    if (m_currentIndex >= 0 && m_currentIndex < m_notificationCount) {
        const Notification& notif = m_notifications[m_currentIndex];
        
        // Draw app icon/badge
        int16_t iconY = SCREEN_CENTER_Y - 60;
        int16_t iconRadius = 30;
        sprite->fillCircle(SCREEN_CENTER_X, iconY, iconRadius, notif.iconColor);
        
        // Draw app name on icon
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextDatum(MC_DATUM);
        sprite->setTextSize(1);
        String appInitial = notif.appName.substring(0, 1);
        appInitial.toUpperCase();
        sprite->drawString(appInitial.c_str(), SCREEN_CENTER_X, iconY);

        // Draw app name below icon
        sprite->setTextColor(TFT_LIGHTGREY);
        sprite->drawString(notif.appName.c_str(), SCREEN_CENTER_X, iconY + iconRadius + 15);

        // Draw title
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextSize(1);
        String title = notif.title;
        if (title.length() > 25) {
            title = title.substring(0, 22) + "...";
        }
        sprite->drawString(title.c_str(), SCREEN_CENTER_X, iconY + iconRadius + 35);

        // Draw message (wrapped)
        sprite->setTextColor(TFT_LIGHTGREY);
        sprite->setTextSize(1);
        String message = notif.message;
        if (message.length() > 60) {
            message = message.substring(0, 57) + "...";
        }
        
        int16_t msgY = iconY + iconRadius + 55;
        int charPerLine = 30;
        for (int i = 0; i < message.length(); i += charPerLine) {
            String line = message.substring(i, min((int)message.length(), i + charPerLine));
            sprite->drawString(line.c_str(), SCREEN_CENTER_X, msgY);
            msgY += 12;
            if (msgY > SCREEN_HEIGHT - 50) break;
        }

        // Draw timestamp
        sprite->setTextColor(TFT_DARKGREY);
        sprite->drawString(notif.timestamp.c_str(), SCREEN_CENTER_X, msgY + 10);

        // Draw read status
        if (notif.isRead) {
            sprite->setTextColor(TFT_GREEN);
            sprite->drawString("✓ Read", SCREEN_CENTER_X, msgY + 25);
        }

        // Draw page indicator
        if (m_notificationCount > 1) {
            char pageStr[16];
            snprintf(pageStr, sizeof(pageStr), "%d / %d", 
                    m_currentIndex + 1, m_notificationCount);
            sprite->setTextColor(TFT_DARKGREY);
            sprite->setTextDatum(BC_DATUM);
            sprite->drawString(pageStr, SCREEN_CENTER_X, SCREEN_HEIGHT - 20);
        }
    }
}

void NotificationView::renderEmptyState() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw checkmark
    sprite->setTextColor(TFT_GREEN);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(4);
    sprite->drawString("✓", SCREEN_CENTER_X, SCREEN_CENTER_Y - 30);

    // Draw message
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextSize(1);
    sprite->drawString("No notifications", SCREEN_CENTER_X, SCREEN_CENTER_Y + 20);

    sprite->setTextColor(TFT_DARKGREY);
    sprite->drawString("You're all caught up!", SCREEN_CENTER_X, SCREEN_CENTER_Y + 40);
}

void NotificationView::handleTouch(TouchEvent event) {
    switch (event) {
        case TouchEvent::TAP:
            if (m_notificationCount > 0) {
                markAsRead(m_currentIndex);
            }
            break;

        case TouchEvent::LONG_PRESS:
            clearAllNotifications();
            break;

        case TouchEvent::SWIPE_LEFT:
            nextNotification();
            break;

        case TouchEvent::SWIPE_RIGHT:
            previousNotification();
            break;

        case TouchEvent::SWIPE_DOWN:
            NavigationController::getInstance().goBack();
            break;

        default:
            break;
    }
}

void NotificationView::loadNotifications() {
    // Clean up old notifications
    if (m_notifications) {
        delete[] m_notifications;
    }

    // Allocate notification array
    const int MAX_NOTIFICATIONS = 20;
    m_notifications = new Notification[MAX_NOTIFICATIONS];
    m_notificationCount = 0;

    // TODO: Load actual notifications from apps
    // For now, create some placeholder notifications
    
    // Example notification from Slack
    m_notifications[m_notificationCount++] = {
        .appName = "Slack",
        .title = "New message",
        .message = "You have a new message from John in #general",
        .timestamp = "2 min ago",
        .iconColor = TFT_PURPLE,
        .isRead = false
    };

    // Example from Spotify
    m_notifications[m_notificationCount++] = {
        .appName = "Spotify",
        .title = "Now playing",
        .message = "Bohemian Rhapsody by Queen",
        .timestamp = "5 min ago",
        .iconColor = TFT_GREEN,
        .isRead = false
    };

    m_currentIndex = 0;
    
    DEBUG_PRINTF("[NotificationView] Loaded %d notifications\n", m_notificationCount);
}

void NotificationView::clearAllNotifications() {
    DEBUG_PRINTLN("[NotificationView] Clearing all notifications");
    
    m_notificationCount = 0;
    m_currentIndex = 0;
    
    // TODO: Actually clear notifications from system
}

void NotificationView::markAsRead(int index) {
    if (index >= 0 && index < m_notificationCount) {
        m_notifications[index].isRead = true;
        DEBUG_PRINTF("[NotificationView] Marked notification %d as read\n", index);
    }
}

void NotificationView::nextNotification() {
    if (m_notificationCount > 0) {
        m_currentIndex = (m_currentIndex + 1) % m_notificationCount;
        DEBUG_PRINTF("[NotificationView] Next: %d\n", m_currentIndex);
    }
}

void NotificationView::previousNotification() {
    if (m_notificationCount > 0) {
        m_currentIndex = (m_currentIndex - 1 + m_notificationCount) % m_notificationCount;
        DEBUG_PRINTF("[NotificationView] Previous: %d\n", m_currentIndex);
    }
}

PageView* createNotificationView() {
    return new NotificationView();
}


