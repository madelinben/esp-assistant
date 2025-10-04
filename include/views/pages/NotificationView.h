/**
 * @file NotificationView.h
 * @brief Notification center view - MVC View Layer
 * 
 * Central notification page showing all app notifications.
 * Part of MVC architecture - View layer (page).
 */

#ifndef NOTIFICATION_VIEW_H
#define NOTIFICATION_VIEW_H

#include <Arduino.h>
#include "controllers/NavigationController.h"
#include "controllers/TouchController.h"
#include "services/AuthService.h"
#include "models/Notification.h"
#include "config/Config.h"

/**
 * @class NotificationView
 * @brief Notification center view page
 * 
 * Features:
 * - List of all notifications from apps
 * - Swipe to navigate notifications
 * - Tap to mark as read
 * - Clear all button
 * - Group by app
 */
class NotificationView : public PageView {
public:
    NotificationView();
    ~NotificationView() override;

    // PageView interface
    void onEnter() override;
    void onExit() override;
    void update() override;
    void render() override;
    void handleTouch(TouchEvent event) override;
    const char* getName() const override { return "Notifications"; }

private:
    void renderNotifications();
    void renderNotification(const Notification& notif, int16_t y);
    void renderEmptyState();
    void loadNotifications();
    void clearAllNotifications();
    void markAsRead(int index);
    void nextNotification();
    void previousNotification();

    Notification* m_notifications;
    int m_notificationCount;
    int m_currentIndex;
    int m_scrollOffset;
    
    TouchPoint m_lastTouch;
};

// Factory function for navigation
PageView* createNotificationView();

#endif // NOTIFICATION_VIEW_H

