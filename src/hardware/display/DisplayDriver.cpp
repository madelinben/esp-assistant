/**
 * @file DisplayDriver.cpp
 * @brief Implementation of DisplayDriver
 */

#include "hardware/display/DisplayDriver.h"

DisplayDriver& DisplayDriver::getInstance() {
    static DisplayDriver instance;
    return instance;
}

DisplayDriver::DisplayDriver() 
    : m_sprite(nullptr)
    , m_initialized(false) {
}

DisplayDriver::~DisplayDriver() {
    if (m_sprite) {
        delete m_sprite;
    }
}

bool DisplayDriver::init() {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[DisplayDriver] Initializing...");

    // Setup backlight PWM
    ledcSetup(0, 5000, 8);  // Channel 0, 5kHz, 8-bit resolution
    ledcAttachPin(TFT_BL, 0);
    
    // Initialize TFT
    m_tft.init();
    m_tft.setRotation(0);
    m_tft.fillScreen(TFT_BLACK);
    
    DEBUG_PRINTLN("[DisplayDriver] TFT initialized");

    // Create sprite for double-buffering in PSRAM
    m_sprite = new TFT_eSprite(&m_tft);
    if (!m_sprite) {
        DEBUG_PRINTLN("[DisplayDriver] ERROR: Failed to create sprite");
        return false;
    }

    // Allocate sprite buffer in PSRAM (large buffer)
    m_sprite->setColorDepth(16);  // RGB565
    bool created = m_sprite->createSprite(WIDTH, HEIGHT);
    
    if (!created) {
        DEBUG_PRINTLN("[DisplayDriver] ERROR: Failed to allocate sprite buffer");
        delete m_sprite;
        m_sprite = nullptr;
        return false;
    }

    DEBUG_PRINTF("[DisplayDriver] Sprite buffer allocated: %dx%d (%d bytes)\n", 
                 WIDTH, HEIGHT, WIDTH * HEIGHT * 2);

    // Set default brightness
    setBrightness(200);  // 80% brightness
    
    // Clear to black
    clear(TFT_BLACK);
    swapBuffers();

    m_initialized = true;
    DEBUG_PRINTLN("[DisplayDriver] Initialized successfully");
    
    return true;
}

void DisplayDriver::setBrightness(uint8_t brightness) {
    ledcWrite(0, brightness);  // PWM channel 0 for backlight
}

void DisplayDriver::swapBuffers() {
    if (m_sprite) {
        m_sprite->pushSprite(0, 0);
    }
}

void DisplayDriver::clear(uint16_t color) {
    if (m_sprite) {
        m_sprite->fillSprite(color);
    }
}

void DisplayDriver::drawCircularBorder(uint16_t color, uint16_t width) {
    if (!m_sprite) return;
    
    // Draw circular border rings
    for (uint16_t i = 0; i < width; i++) {
        m_sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, RADIUS - i, color);
    }
}

void DisplayDriver::drawCurvedText(const char* text, float angleStart, float radius, uint16_t color) {
    if (!m_sprite || !text) return;
    
    m_sprite->setTextColor(color);
    m_sprite->setTextDatum(MC_DATUM);  // Middle center
    
    size_t len = strlen(text);
    float angleStep = 15.0f;  // Degrees between characters
    float currentAngle = angleStart;
    
    for (size_t i = 0; i < len; i++) {
        float rad = currentAngle * DEG_TO_RAD;
        int16_t x = SCREEN_CENTER_X + (int16_t)(radius * sin(rad));
        int16_t y = SCREEN_CENTER_Y - (int16_t)(radius * cos(rad));
        
        char str[2] = {text[i], '\0'};
        m_sprite->drawString(str, x, y);
        
        currentAngle += angleStep;
    }
}

bool DisplayDriver::isInsideCircle(int16_t x, int16_t y) const {
    int16_t dx = x - SCREEN_CENTER_X;
    int16_t dy = y - SCREEN_CENTER_Y;
    return (dx * dx + dy * dy) <= (RADIUS * RADIUS);
}

void DisplayDriver::drawPixelClipped(int16_t x, int16_t y, uint16_t color) {
    if (m_sprite && isInsideCircle(x, y)) {
        m_sprite->drawPixel(x, y, color);
    }
}

void DisplayDriver::fillCircleClipped(int16_t x, int16_t y, int16_t r, uint16_t color) {
    if (!m_sprite) return;
    
    // Only draw pixels inside the circular display bounds
    for (int16_t dy = -r; dy <= r; dy++) {
        for (int16_t dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                int16_t px = x + dx;
                int16_t py = y + dy;
                if (isInsideCircle(px, py)) {
                    m_sprite->drawPixel(px, py, color);
                }
            }
        }
    }
}

