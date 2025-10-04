/**
 * @file TouchController.h
 * @brief Touch input controller - MVC Controller Layer
 * 
 * Processes touch input from hardware and generates high-level events.
 * Part of MVC architecture - Controller layer.
 */

#ifndef TOUCH_CONTROLLER_H
#define TOUCH_CONTROLLER_H

#include <Arduino.h>
#include "hardware/touch/TouchDriver.h"

/**
 * @enum TouchEvent
 * @brief High-level touch events
 */
enum class TouchEvent {
    NONE,
    TAP,
    DOUBLE_TAP,
    LONG_PRESS,
    DRAG_START,
    DRAG_MOVE,
    DRAG_END,
    SWIPE_UP,
    SWIPE_DOWN,
    SWIPE_LEFT,
    SWIPE_RIGHT
};

/**
 * @struct TouchPoint
 * @brief Processed touch point data
 */
struct TouchPoint {
    int16_t x;
    int16_t y;
    bool pressed;
    uint32_t timestamp;
};

/**
 * @class TouchController
 * @brief Controller for touch input processing
 * 
 * Converts raw touch data from HAL into high-level events.
 * Handles gesture detection and touch state management.
 */
class TouchController {
public:
    /**
     * @brief Get singleton instance
     */
    static TouchController& getInstance();

    /**
     * @brief Initialize controller
     */
    bool init();

    /**
     * @brief Update touch state (call in main loop)
     */
    void update();

    /**
     * @brief Get current touch point
     */
    TouchPoint getCurrentTouch() const { return m_currentTouch; }

    /**
     * @brief Get last detected event
     */
    TouchEvent getLastEvent() const { return m_lastEvent; }

    /**
     * @brief Check if position is within circular display
     */
    bool isInsideCircle(int16_t x, int16_t y) const;

private:
    TouchController();
    ~TouchController();
    TouchController(const TouchController&) = delete;
    TouchController& operator=(const TouchController&) = delete;

    void detectGestures();
    void resetGestureState();

    TouchPoint m_currentTouch;
    TouchPoint m_lastTouch;
    TouchPoint m_touchStart;
    TouchEvent m_lastEvent;
    uint32_t m_touchStartTime;
    bool m_isDragging;
};

#endif // TOUCH_CONTROLLER_H


