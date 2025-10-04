/**
 * @file TouchController.cpp
 * @brief Implementation of TouchController
 */

#include "controllers/TouchController.h"
#include "config/Config.h"

#define TAP_MAX_DURATION 300
#define LONG_PRESS_DURATION 800
#define DRAG_THRESHOLD 10
#define SWIPE_THRESHOLD 50

TouchController& TouchController::getInstance() {
    static TouchController instance;
    return instance;
}

TouchController::TouchController()
    : m_lastEvent(TouchEvent::NONE)
    , m_touchStartTime(0)
    , m_isDragging(false) {
    m_currentTouch = {0, 0, false, 0};
    m_lastTouch = {0, 0, false, 0};
    m_touchStart = {0, 0, false, 0};
}

TouchController::~TouchController() {
}

bool TouchController::init() {
    return TouchDriver::getInstance().init();
}

void TouchController::update() {
    m_lastEvent = TouchEvent::NONE;
    m_lastTouch = m_currentTouch;

    TouchData rawData;
    bool touched = TouchDriver::getInstance().read(rawData);

    m_currentTouch.x = rawData.x;
    m_currentTouch.y = rawData.y;
    m_currentTouch.pressed = touched;
    m_currentTouch.timestamp = millis();

    if (touched && !m_lastTouch.pressed) {
        // Touch started
        m_touchStart = m_currentTouch;
        m_touchStartTime = millis();
        m_isDragging = false;
    } else if (touched && m_lastTouch.pressed) {
        // Touch continuing
        detectGestures();
    } else if (!touched && m_lastTouch.pressed) {
        // Touch ended
        uint32_t duration = millis() - m_touchStartTime;
        
        if (m_isDragging) {
            m_lastEvent = TouchEvent::DRAG_END;
        } else if (duration < TAP_MAX_DURATION) {
            m_lastEvent = TouchEvent::TAP;
        }
        
        resetGestureState();
    }
}

void TouchController::detectGestures() {
    int16_t deltaX = m_currentTouch.x - m_touchStart.x;
    int16_t deltaY = m_currentTouch.y - m_touchStart.y;
    uint32_t duration = millis() - m_touchStartTime;

    // Long press detection
    if (!m_isDragging && duration > LONG_PRESS_DURATION) {
        if (abs(deltaX) < DRAG_THRESHOLD && abs(deltaY) < DRAG_THRESHOLD) {
            m_lastEvent = TouchEvent::LONG_PRESS;
            return;
        }
    }

    // Drag detection
    if (abs(deltaX) > DRAG_THRESHOLD || abs(deltaY) > DRAG_THRESHOLD) {
        if (!m_isDragging) {
            m_lastEvent = TouchEvent::DRAG_START;
            m_isDragging = true;
        } else {
            m_lastEvent = TouchEvent::DRAG_MOVE;
        }
    }
}

void TouchController::resetGestureState() {
    m_isDragging = false;
    m_touchStartTime = 0;
}

bool TouchController::isInsideCircle(int16_t x, int16_t y) const {
    int16_t dx = x - SCREEN_CENTER_X;
    int16_t dy = y - SCREEN_CENTER_Y;
    return (dx * dx + dy * dy) <= (SCREEN_RADIUS * SCREEN_RADIUS);
}


