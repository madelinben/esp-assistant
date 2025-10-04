/**
 * @file FrameBuffer.cpp
 * @brief Implementation of FrameBuffer
 */

#include "hardware/display/FrameBuffer.h"

FrameBuffer::FrameBuffer(TFT_eSPI* tft)
    : m_tft(tft)
    , m_frontBuffer(nullptr)
    , m_backBuffer(nullptr)
    , m_dirtyTracking(true)
    , m_frameCount(0)
    , m_frameTime(0)
    , m_lastFrameTime(0)
    , m_initialized(false) {
    
    m_dirtyRegion = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, true};
}

FrameBuffer::~FrameBuffer() {
    if (m_frontBuffer) {
        m_frontBuffer->deleteSprite();
        delete m_frontBuffer;
    }
    if (m_backBuffer) {
        m_backBuffer->deleteSprite();
        delete m_backBuffer;
    }
}

bool FrameBuffer::init() {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[FrameBuffer] Initializing double-buffering...");

    if (!m_tft) {
        DEBUG_PRINTLN("[FrameBuffer] ERROR: TFT instance is null");
        return false;
    }

    // Create front buffer sprite
    m_frontBuffer = new TFT_eSprite(m_tft);
    if (!m_frontBuffer) {
        DEBUG_PRINTLN("[FrameBuffer] ERROR: Failed to create front buffer");
        return false;
    }

    m_frontBuffer->setColorDepth(16);  // RGB565
    if (!m_frontBuffer->createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        DEBUG_PRINTLN("[FrameBuffer] ERROR: Failed to allocate front buffer memory");
        delete m_frontBuffer;
        m_frontBuffer = nullptr;
        return false;
    }

    // Create back buffer sprite
    m_backBuffer = new TFT_eSprite(m_tft);
    if (!m_backBuffer) {
        DEBUG_PRINTLN("[FrameBuffer] ERROR: Failed to create back buffer");
        m_frontBuffer->deleteSprite();
        delete m_frontBuffer;
        m_frontBuffer = nullptr;
        return false;
    }

    m_backBuffer->setColorDepth(16);  // RGB565
    if (!m_backBuffer->createSprite(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        DEBUG_PRINTLN("[FrameBuffer] ERROR: Failed to allocate back buffer memory");
        m_frontBuffer->deleteSprite();
        delete m_frontBuffer;
        delete m_backBuffer;
        m_frontBuffer = nullptr;
        m_backBuffer = nullptr;
        return false;
    }

    // Clear both buffers
    m_frontBuffer->fillSprite(TFT_BLACK);
    m_backBuffer->fillSprite(TFT_BLACK);

    // Calculate buffer size
    size_t bufferSize = SCREEN_WIDTH * SCREEN_HEIGHT * 2;  // 2 bytes per pixel (RGB565)
    DEBUG_PRINTF("[FrameBuffer] Buffer size: %d KB per buffer\n", bufferSize / 1024);
    DEBUG_PRINTF("[FrameBuffer] Total memory: %d KB (2 buffers)\n", (bufferSize * 2) / 1024);

    m_initialized = true;
    m_lastFrameTime = millis();

    DEBUG_PRINTLN("[FrameBuffer] Double-buffering initialized successfully");
    return true;
}

void FrameBuffer::beginFrame() {
    if (!m_initialized) return;

    // Calculate frame time
    uint32_t currentTime = millis();
    m_frameTime = currentTime - m_lastFrameTime;
    m_lastFrameTime = currentTime;

    // Back buffer is ready for drawing
    // All drawing operations should use m_backBuffer
}

void FrameBuffer::endFrame(bool fullUpdate) {
    if (!m_initialized) return;

    // Swap buffers
    swapBuffers();

    // Push to display
    if (fullUpdate || !m_dirtyTracking) {
        // Full screen update
        m_frontBuffer->pushSprite(0, 0);
    } else {
        // Partial update (only dirty region)
        if (m_dirtyRegion.dirty) {
            m_frontBuffer->pushSprite(m_dirtyRegion.x, m_dirtyRegion.y,
                                     m_dirtyRegion.width, m_dirtyRegion.height);
            m_dirtyRegion.dirty = false;
        }
    }

    m_frameCount++;
}

void FrameBuffer::swapBuffers() {
    // Swap front and back buffer pointers
    TFT_eSprite* temp = m_frontBuffer;
    m_frontBuffer = m_backBuffer;
    m_backBuffer = temp;
}

void FrameBuffer::clear(uint16_t color) {
    if (m_backBuffer) {
        m_backBuffer->fillSprite(color);
    }
}

void FrameBuffer::markDirty(int16_t x, int16_t y, int16_t width, int16_t height) {
    if (!m_dirtyTracking) return;

    // Clamp to screen bounds
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;

    if (width <= 0 || height <= 0) return;

    // Expand dirty region to include this area
    if (!m_dirtyRegion.dirty) {
        m_dirtyRegion.x = x;
        m_dirtyRegion.y = y;
        m_dirtyRegion.width = width;
        m_dirtyRegion.height = height;
        m_dirtyRegion.dirty = true;
    } else {
        // Merge with existing dirty region
        int16_t x1 = min(m_dirtyRegion.x, x);
        int16_t y1 = min(m_dirtyRegion.y, y);
        int16_t x2 = max(m_dirtyRegion.x + m_dirtyRegion.width, x + width);
        int16_t y2 = max(m_dirtyRegion.y + m_dirtyRegion.height, y + height);
        
        m_dirtyRegion.x = x1;
        m_dirtyRegion.y = y1;
        m_dirtyRegion.width = x2 - x1;
        m_dirtyRegion.height = y2 - y1;
    }
}

void FrameBuffer::markAllDirty() {
    m_dirtyRegion.x = 0;
    m_dirtyRegion.y = 0;
    m_dirtyRegion.width = SCREEN_WIDTH;
    m_dirtyRegion.height = SCREEN_HEIGHT;
    m_dirtyRegion.dirty = true;
}

bool FrameBuffer::isDirty(int16_t x, int16_t y, int16_t width, int16_t height) {
    if (!m_dirtyTracking || !m_dirtyRegion.dirty) return false;

    // Check if regions overlap
    return !(x + width < m_dirtyRegion.x ||
             x > m_dirtyRegion.x + m_dirtyRegion.width ||
             y + height < m_dirtyRegion.y ||
             y > m_dirtyRegion.y + m_dirtyRegion.height);
}

void FrameBuffer::clearDirtyFlags() {
    m_dirtyRegion.dirty = false;
}

float FrameBuffer::getFPS() const {
    if (m_frameTime == 0) return 0.0f;
    return 1000.0f / (float)m_frameTime;
}


