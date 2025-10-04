/**
 * @file CircularSlider.h
 * @brief Circular slider UI component - MVC View Layer
 * 
 * Reusable circular slider for volume, brightness, duration controls.
 * Part of MVC architecture - View layer (component).
 */

#ifndef CIRCULAR_SLIDER_H
#define CIRCULAR_SLIDER_H

#include <Arduino.h>
#include "hardware/display/DisplayDriver.h"
#include "controllers/TouchController.h"

/**
 * @enum SliderMode
 * @brief Slider display modes
 */
enum class SliderMode {
    VOLUME,
    BRIGHTNESS,
    DURATION,
    HUE,
    TEMPERATURE,
    GENERIC
};

/**
 * @class CircularSlider
 * @brief Circular slider component
 * 
 * Features:
 * - Touch-based circular dragging
 * - Visual arc representation of value
 * - Configurable range (min/max)
 * - Multiple display modes
 * - Value change callbacks
 * - Smooth visual updates
 * - Center icon/text display
 */
class CircularSlider {
public:
    /**
     * @brief Constructor
     * @param centerX Center X coordinate
     * @param centerY Center Y coordinate
     * @param radius Slider radius
     * @param innerRadius Inner radius (for arc thickness)
     */
    CircularSlider(int16_t centerX, int16_t centerY, int16_t radius, int16_t innerRadius);

    /**
     * @brief Destructor
     */
    ~CircularSlider();

    /**
     * @brief Set value range
     * @param min Minimum value
     * @param max Maximum value
     */
    void setRange(float min, float max);

    /**
     * @brief Set current value
     * @param value Current value
     */
    void setValue(float value);

    /**
     * @brief Get current value
     * @return Current value
     */
    float getValue() const { return m_value; }

    /**
     * @brief Set slider mode
     * @param mode Display mode
     */
    void setMode(SliderMode mode);

    /**
     * @brief Set colors
     * @param activeColor Color for filled arc
     * @param inactiveColor Color for unfilled arc
     * @param textColor Text color
     */
    void setColors(uint32_t activeColor, uint32_t inactiveColor, uint32_t textColor);

    /**
     * @brief Set label text
     * @param label Label to display
     */
    void setLabel(const String& label);

    /**
     * @brief Set value change callback
     * @param callback Function to call when value changes
     */
    void setOnValueChanged(void (*callback)(float value));

    /**
     * @brief Enable/disable slider
     * @param enabled true to enable
     */
    void setEnabled(bool enabled);

    /**
     * @brief Check if enabled
     * @return true if enabled
     */
    bool isEnabled() const { return m_enabled; }

    /**
     * @brief Update slider (call each frame)
     * @param touchPoint Current touch point
     */
    void update(const TouchPoint& touchPoint);

    /**
     * @brief Render slider
     */
    void render();

    /**
     * @brief Check if touch is within slider bounds
     * @param x Touch X coordinate
     * @param y Touch Y coordinate
     * @return true if within bounds
     */
    bool contains(int16_t x, int16_t y) const;

private:
    void calculateAngleFromTouch(int16_t touchX, int16_t touchY);
    void calculateValueFromAngle();
    void drawArc(int16_t cx, int16_t cy, int16_t r, int16_t startAngle, int16_t endAngle, uint32_t color, int16_t thickness);
    void drawCenterContent();
    String formatValue() const;

    // Position and size
    int16_t m_centerX;
    int16_t m_centerY;
    int16_t m_radius;
    int16_t m_innerRadius;

    // Value properties
    float m_value;
    float m_minValue;
    float m_maxValue;
    float m_targetValue;  // For smooth animation

    // Angle properties (0-360 degrees, 0 = top)
    int16_t m_startAngle;  // Start angle for arc (default: 135)
    int16_t m_endAngle;    // End angle for arc (default: 45, 270 degree range)
    int16_t m_currentAngle;

    // Visual properties
    uint32_t m_activeColor;
    uint32_t m_inactiveColor;
    uint32_t m_textColor;
    String m_label;
    SliderMode m_mode;

    // State
    bool m_enabled;
    bool m_isDragging;
    bool m_hasChanged;

    // Callback
    void (*m_onValueChanged)(float value);
};

#endif // CIRCULAR_SLIDER_H
