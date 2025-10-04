/**
 * @file CircularSlider.cpp
 * @brief Implementation of CircularSlider
 */

#include "views/components/CircularSlider.h"
#include <cmath>

// Helper function to normalize angle to 0-360 range
static int16_t normalizeAngle(int16_t angle) {
    while (angle < 0) angle += 360;
    while (angle >= 360) angle -= 360;
    return angle;
}

// Helper function to calculate distance between two points
static float distance(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

CircularSlider::CircularSlider(int16_t centerX, int16_t centerY, int16_t radius, int16_t innerRadius)
    : m_centerX(centerX)
    , m_centerY(centerY)
    , m_radius(radius)
    , m_innerRadius(innerRadius)
    , m_value(0.0f)
    , m_minValue(0.0f)
    , m_maxValue(100.0f)
    , m_targetValue(0.0f)
    , m_startAngle(135)    // Bottom-left (7 o'clock)
    , m_endAngle(45)       // Bottom-right (5 o'clock), giving 270° range
    , m_currentAngle(135)
    , m_activeColor(TFT_CYAN)
    , m_inactiveColor(TFT_DARKGREY)
    , m_textColor(TFT_WHITE)
    , m_label("")
    , m_mode(SliderMode::GENERIC)
    , m_enabled(true)
    , m_isDragging(false)
    , m_hasChanged(false)
    , m_onValueChanged(nullptr) {
}

CircularSlider::~CircularSlider() {
}

void CircularSlider::setRange(float min, float max) {
    m_minValue = min;
    m_maxValue = max;
    
    // Clamp current value to new range
    if (m_value < m_minValue) m_value = m_minValue;
    if (m_value > m_maxValue) m_value = m_maxValue;
    
    calculateValueFromAngle();
}

void CircularSlider::setValue(float value) {
    // Clamp value to range
    if (value < m_minValue) value = m_minValue;
    if (value > m_maxValue) value = m_maxValue;
    
    if (m_value != value) {
        m_value = value;
        m_targetValue = value;
        
        // Calculate angle from value
        float normalizedValue = (m_value - m_minValue) / (m_maxValue - m_minValue);
        
        // Map to angle range (270 degrees)
        int16_t angleRange = 270;
        m_currentAngle = m_startAngle + (int16_t)(normalizedValue * angleRange);
        m_currentAngle = normalizeAngle(m_currentAngle);
        
        m_hasChanged = true;
    }
}

void CircularSlider::setMode(SliderMode mode) {
    m_mode = mode;
    
    // Set default colors based on mode
    switch (mode) {
        case SliderMode::VOLUME:
            m_activeColor = TFT_GREEN;
            m_label = "Volume";
            break;
        case SliderMode::BRIGHTNESS:
            m_activeColor = TFT_YELLOW;
            m_label = "Brightness";
            break;
        case SliderMode::DURATION:
            m_activeColor = TFT_CYAN;
            m_label = "Duration";
            break;
        case SliderMode::HUE:
            m_activeColor = TFT_MAGENTA;
            m_label = "Hue";
            break;
        case SliderMode::TEMPERATURE:
            m_activeColor = TFT_ORANGE;
            m_label = "Temperature";
            break;
        default:
            m_activeColor = TFT_CYAN;
            m_label = "";
            break;
    }
}

void CircularSlider::setColors(uint32_t activeColor, uint32_t inactiveColor, uint32_t textColor) {
    m_activeColor = activeColor;
    m_inactiveColor = inactiveColor;
    m_textColor = textColor;
}

void CircularSlider::setLabel(const String& label) {
    m_label = label;
}

void CircularSlider::setOnValueChanged(void (*callback)(float value)) {
    m_onValueChanged = callback;
}

void CircularSlider::setEnabled(bool enabled) {
    m_enabled = enabled;
    if (!enabled) {
        m_isDragging = false;
    }
}

void CircularSlider::update(const TouchPoint& touchPoint) {
    if (!m_enabled) {
        return;
    }

    bool isTouching = touchPoint.pressed;
    bool touchInBounds = contains(touchPoint.x, touchPoint.y);

    if (isTouching && touchInBounds) {
        if (!m_isDragging) {
            m_isDragging = true;
        }
        
        // Update angle based on touch position
        calculateAngleFromTouch(touchPoint.x, touchPoint.y);
        calculateValueFromAngle();
        
        m_hasChanged = true;
        
        // Call callback if value changed
        if (m_onValueChanged) {
            m_onValueChanged(m_value);
        }
    } else if (m_isDragging && !isTouching) {
        m_isDragging = false;
    }

    // Smooth value animation (optional)
    if (m_value != m_targetValue) {
        float diff = m_targetValue - m_value;
        m_value += diff * 0.2f;  // Smooth interpolation
        
        if (fabs(diff) < 0.1f) {
            m_value = m_targetValue;
        }
    }
}

void CircularSlider::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw inactive arc (full range)
    drawArc(m_centerX, m_centerY, m_radius, m_startAngle, m_endAngle, m_inactiveColor, m_radius - m_innerRadius);

    // Draw active arc (current value)
    if (m_value > m_minValue) {
        drawArc(m_centerX, m_centerY, m_radius, m_startAngle, m_currentAngle, m_activeColor, m_radius - m_innerRadius);
    }

    // Draw center content
    drawCenterContent();

    // Draw drag indicator if dragging
    if (m_isDragging) {
        // Calculate position at current angle
        float angleRad = (m_currentAngle - 90) * DEG_TO_RAD;
        int16_t indicatorX = m_centerX + (int16_t)(m_radius * cos(angleRad));
        int16_t indicatorY = m_centerY + (int16_t)(m_radius * sin(angleRad));
        
        sprite->fillCircle(indicatorX, indicatorY, 8, m_activeColor);
        sprite->drawCircle(indicatorX, indicatorY, 8, TFT_WHITE);
    }

    m_hasChanged = false;
}

bool CircularSlider::contains(int16_t x, int16_t y) const {
    float dist = distance(m_centerX, m_centerY, x, y);
    return (dist >= m_innerRadius && dist <= m_radius);
}

void CircularSlider::calculateAngleFromTouch(int16_t touchX, int16_t touchY) {
    // Calculate angle from center to touch point
    int16_t dx = touchX - m_centerX;
    int16_t dy = touchY - m_centerY;
    
    // Calculate angle in degrees (0 = right, 90 = down, 180 = left, 270 = up)
    float angleRad = atan2(dy, dx);
    int16_t angleDeg = (int16_t)(angleRad * RAD_TO_DEG);
    
    // Convert to our coordinate system (0 = up)
    angleDeg += 90;
    angleDeg = normalizeAngle(angleDeg);
    
    // Clamp to valid range (135° to 45° = 270° range)
    // Map to 0-270 range
    int16_t relativeAngle;
    if (angleDeg >= m_startAngle) {
        relativeAngle = angleDeg - m_startAngle;
    } else {
        relativeAngle = (360 - m_startAngle) + angleDeg;
    }
    
    // Clamp to 270 degree range
    if (relativeAngle > 270) {
        // Snap to nearest end
        if (relativeAngle > 315) {
            relativeAngle = 270;
        } else {
            relativeAngle = 0;
        }
    }
    
    m_currentAngle = normalizeAngle(m_startAngle + relativeAngle);
}

void CircularSlider::calculateValueFromAngle() {
    // Calculate relative angle (0-270)
    int16_t relativeAngle;
    if (m_currentAngle >= m_startAngle) {
        relativeAngle = m_currentAngle - m_startAngle;
    } else {
        relativeAngle = (360 - m_startAngle) + m_currentAngle;
    }
    
    // Normalize to 0-1 range
    float normalizedValue = (float)relativeAngle / 270.0f;
    
    // Map to value range
    m_targetValue = m_minValue + (normalizedValue * (m_maxValue - m_minValue));
    m_value = m_targetValue;
}

void CircularSlider::drawArc(int16_t cx, int16_t cy, int16_t r, int16_t startAngle, int16_t endAngle, uint32_t color, int16_t thickness) {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Calculate angle range
    int16_t angleRange;
    if (endAngle >= startAngle) {
        angleRange = endAngle - startAngle;
    } else {
        angleRange = (360 - startAngle) + endAngle;
    }
    
    // Draw arc using small line segments
    int16_t segments = angleRange / 2;  // 2 degrees per segment
    if (segments < 1) segments = 1;
    
    for (int16_t i = 0; i <= segments; i++) {
        float t = (float)i / (float)segments;
        int16_t angle = startAngle + (int16_t)(t * angleRange);
        angle = normalizeAngle(angle);
        
        float angleRad = (angle - 90) * DEG_TO_RAD;
        
        // Draw thick arc by drawing multiple circles at different radii
        for (int16_t j = 0; j < thickness; j++) {
            int16_t currentR = r - j;
            int16_t x = cx + (int16_t)(currentR * cos(angleRad));
            int16_t y = cy + (int16_t)(currentR * sin(angleRad));
            sprite->drawPixel(x, y, color);
        }
    }
}

void CircularSlider::drawCenterContent() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Format and draw value
    String valueStr = formatValue();
    
    sprite->setTextColor(m_textColor);
    sprite->setTextDatum(MC_DATUM);
    
    // Draw value (large font)
    sprite->setTextSize(2);
    sprite->drawString(valueStr, m_centerX, m_centerY);
    
    // Draw label (small font)
    if (m_label.length() > 0) {
        sprite->setTextSize(1);
        sprite->setTextColor(m_inactiveColor);
        sprite->drawString(m_label, m_centerX, m_centerY + 20);
    }
}

String CircularSlider::formatValue() const {
    String result;
    
    switch (m_mode) {
        case SliderMode::VOLUME:
            result = String((int)m_value) + "%";
            break;
        case SliderMode::BRIGHTNESS:
            result = String((int)m_value) + "%";
            break;
        case SliderMode::DURATION:
            // Format as time (MM:SS)
            {
                int totalSeconds = (int)m_value;
                int minutes = totalSeconds / 60;
                int seconds = totalSeconds % 60;
                result = String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
            }
            break;
        case SliderMode::HUE:
            result = String((int)m_value) + "°";
            break;
        case SliderMode::TEMPERATURE:
            result = String(m_value, 1) + "°C";
            break;
        default:
            result = String((int)m_value);
            break;
    }
    
    return result;
}


