/**
 * @file SlackView.cpp
 * @brief Implementation of SlackView
 */

#include "views/apps/slack/SlackView.h"
#include "hardware/display/DisplayDriver.h"

SlackView::SlackView()
    : m_controller(nullptr)
    , m_notifications(nullptr)
    , m_notificationCount(0)
    , m_currentIndex(0)
    , m_lastUpdate(0) {
    
    m_isActive = false;
    m_lastTouch = {0, 0, false, 0};
}

SlackView::~SlackView() {
    if (m_notifications) {
        delete[] m_notifications;
    }
}

void SlackView::onEnter() {
    DEBUG_PRINTLN("[SlackView] Entering...");
    
    m_isActive = true;
    m_controller = &SlackController::getInstance();
    
    // Load notifications
    loadNotifications();
    
    DEBUG_PRINTF("[SlackView] Loaded %d notifications\n", m_notificationCount);
}

void SlackView::onExit() {
    DEBUG_PRINTLN("[SlackView] Exiting...");
    m_isActive = false;
}

void SlackView::update() {
    // Refresh notifications every 30 seconds
    uint32_t currentTime = millis();
    if (currentTime - m_lastUpdate >= 30000) {
        loadNotifications();
        m_lastUpdate = currentTime;
    }
}

void SlackView::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Clear background
    sprite->fillSprite(TFT_BLACK);

    // Draw circular border
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, TFT_PURPLE);
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS - 1, TFT_PURPLE);

    // Draw title
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString("SLACK", SCREEN_CENTER_X, 15);

    // Draw notification count in top-right
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

    // Render notification or empty state
    if (m_notificationCount > 0 && m_currentIndex < m_notificationCount) {
        renderNotification(m_notifications[m_currentIndex]);
        
        // Draw pagination indicators
        if (m_notificationCount > 1) {
            sprite->setTextColor(TFT_DARKGREY);
            sprite->setTextDatum(BC_DATUM);
            char pageStr[16];
            snprintf(pageStr, sizeof(pageStr), "%d / %d", m_currentIndex + 1, m_notificationCount);
            sprite->drawString(pageStr, SCREEN_CENTER_X, SCREEN_HEIGHT - 15);
        }
    } else {
        renderNoNotifications();
    }

    // Draw swipe hint
    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString("Swipe left/right • Tap to dismiss", SCREEN_CENTER_X, SCREEN_HEIGHT - 5);
}

void SlackView::renderNotification(const SlackNotification& notif) {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw notification icon in center
    int16_t iconRadius = 40;
    int16_t iconY = SCREEN_CENTER_Y - 30;
    
    // Icon background based on type
    uint16_t iconColor = TFT_BLUE;
    switch (notif.type) {
        case SlackNotificationType::MESSAGE:
            iconColor = TFT_BLUE;
            break;
        case SlackNotificationType::MENTION:
            iconColor = TFT_RED;
            break;
        case SlackNotificationType::CALL:
            iconColor = TFT_GREEN;
            break;
        case SlackNotificationType::CHANNEL:
            iconColor = TFT_PURPLE;
            break;
        default:
            iconColor = TFT_DARKGREY;
    }
    
    sprite->fillCircle(SCREEN_CENTER_X, iconY, iconRadius, iconColor);
    
    // Draw icon symbol (simplified)
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(3);
    
    const char* symbol = "?";
    switch (notif.type) {
        case SlackNotificationType::MESSAGE:
            symbol = "M";
            break;
        case SlackNotificationType::MENTION:
            symbol = "@";
            break;
        case SlackNotificationType::CALL:
            symbol = "C";
            break;
        case SlackNotificationType::CHANNEL:
            symbol = "#";
            break;
    }
    sprite->drawString(symbol, SCREEN_CENTER_X, iconY);

    // Draw channel/sender name
    sprite->setTextColor(TFT_LIGHTGREY);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString(notif.channelName.c_str(), SCREEN_CENTER_X, iconY + iconRadius + 20);

    // Draw message text (wrapped)
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    
    String message = notif.text;
    if (message.length() > 60) {
        message = message.substring(0, 57) + "...";
    }
    
    // Word wrap at ~30 chars per line
    int16_t textY = iconY + iconRadius + 45;
    int charPerLine = 30;
    for (int i = 0; i < message.length(); i += charPerLine) {
        String line = message.substring(i, min((int)message.length(), i + charPerLine));
        sprite->drawString(line.c_str(), SCREEN_CENTER_X, textY);
        textY += 15;
        if (textY > SCREEN_HEIGHT - 50) break; // Don't overflow
    }

    // Draw timestamp
    sprite->setTextColor(TFT_DARKGREY);
    sprite->setTextDatum(TC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString(notif.timestamp.c_str(), SCREEN_CENTER_X, textY + 10);
}

void SlackView::renderNoNotifications() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw large checkmark or icon
    sprite->setTextColor(TFT_GREEN);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(4);
    sprite->drawString("✓", SCREEN_CENTER_X, SCREEN_CENTER_Y - 30);

    // Draw "All caught up" message
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(MC_DATUM);
    sprite->setTextSize(1);
    sprite->drawString("All caught up!", SCREEN_CENTER_X, SCREEN_CENTER_Y + 20);

    sprite->setTextColor(TFT_DARKGREY);
    sprite->drawString("No new Slack notifications", SCREEN_CENTER_X, SCREEN_CENTER_Y + 40);
}

void SlackView::handleTouch(TouchEvent event) {
    TouchController& touchCtrl = TouchController::getInstance();
    TouchPoint currentTouch = touchCtrl.getCurrentTouch();

    switch (event) {
        case TouchEvent::TAP:
            // Dismiss current notification
            if (m_notificationCount > 0) {
                dismissCurrentNotification();
            }
            break;

        case TouchEvent::SWIPE_LEFT:
            // Next notification
            nextNotification();
            break;

        case TouchEvent::SWIPE_RIGHT:
            // Previous notification
            previousNotification();
            break;

        case TouchEvent::SWIPE_DOWN:
            // Go back to main menu
            NavigationController::getInstance().goBack();
            break;

        default:
            break;
    }

    m_lastTouch = currentTouch;
}

void SlackView::loadNotifications() {
    if (!m_controller) return;

    // Clean up old notifications
    if (m_notifications) {
        delete[] m_notifications;
        m_notifications = nullptr;
    }

    // Allocate array for notifications
    const int MAX_NOTIFICATIONS = 20;
    m_notifications = new SlackNotification[MAX_NOTIFICATIONS];

    // Get notifications from controller
    m_notificationCount = m_controller->getNotifications(m_notifications, MAX_NOTIFICATIONS);
    m_currentIndex = 0;
    m_lastUpdate = millis();

    DEBUG_PRINTF("[SlackView] Loaded %d notifications\n", m_notificationCount);
}

void SlackView::nextNotification() {
    if (m_notificationCount > 0) {
        m_currentIndex = (m_currentIndex + 1) % m_notificationCount;
        DEBUG_PRINTF("[SlackView] Next notification: %d\n", m_currentIndex);
    }
}

void SlackView::previousNotification() {
    if (m_notificationCount > 0) {
        m_currentIndex = (m_currentIndex - 1 + m_notificationCount) % m_notificationCount;
        DEBUG_PRINTF("[SlackView] Previous notification: %d\n", m_currentIndex);
    }
}

void SlackView::dismissCurrentNotification() {
    if (m_notificationCount == 0 || !m_controller) return;

    DEBUG_PRINTF("[SlackView] Dismissing notification: %s\n", 
                 m_notifications[m_currentIndex].channelName.c_str());

    // Mark as read (TODO: implement in controller)
    // m_controller->markAsRead(m_notifications[m_currentIndex].id);

    // Remove from array
    for (int i = m_currentIndex; i < m_notificationCount - 1; i++) {
        m_notifications[i] = m_notifications[i + 1];
    }
    m_notificationCount--;

    // Adjust current index
    if (m_currentIndex >= m_notificationCount) {
        m_currentIndex = max(0, m_notificationCount - 1);
    }

    DEBUG_PRINTF("[SlackView] %d notifications remaining\n", m_notificationCount);
}

PageView* createSlackView() {
    return new SlackView();
}


