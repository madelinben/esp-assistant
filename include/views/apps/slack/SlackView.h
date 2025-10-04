/**
 * @file SlackView.h
 * @brief Slack notification view - MVC View Layer
 * 
 * Apple Watch inspired notification display for Slack.
 * Part of MVC architecture - View layer (app view).
 */

#ifndef SLACK_VIEW_H
#define SLACK_VIEW_H

#include <Arduino.h>
#include "controllers/NavigationController.h"
#include "controllers/TouchController.h"
#include "controllers/apps/slack/SlackController.h"
#include "models/slack/SlackNotification.h"
#include "services/AuthService.h"
#include "config/Config.h"

/**
 * @class SlackView
 * @brief Slack notification view page
 * 
 * Features:
 * - Center icon/image for notification
 * - Text below showing update
 * - Notification count in top-right
 * - Swipe to navigate notifications
 * - Tap to open/dismiss
 */
class SlackView : public PageView {
public:
    SlackView();
    ~SlackView() override;

    // PageView interface
    void onEnter() override;
    void onExit() override;
    void update() override;
    void render() override;
    void handleTouch(TouchEvent event) override;
    const char* getName() const override { return "Slack"; }

private:
    void renderNotification(const SlackNotification& notif);
    void renderNoNotifications();
    void loadNotifications();
    void nextNotification();
    void previousNotification();
    void dismissCurrentNotification();

    SlackController* m_controller;
    SlackNotification* m_notifications;
    int m_notificationCount;
    int m_currentIndex;
    uint32_t m_lastUpdate;
    
    TouchPoint m_lastTouch;
};

// Factory function for navigation
PageView* createSlackView();

#endif // SLACK_VIEW_H


