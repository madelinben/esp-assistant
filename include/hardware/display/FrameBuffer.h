/**
 * @file FrameBuffer.h
 * @brief Frame buffer with double-buffering support
 * 
 * Manages double-buffering for flicker-free display updates.
 * Implements dirty region tracking for optimized rendering.
 * Part of Hardware Abstraction Layer (HAL).
 */

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "config/Config.h"

/**
 * @struct DirtyRegion
 * @brief Region that needs redrawing
 */
struct DirtyRegion {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    bool dirty;
};

/**
 * @class FrameBuffer
 * @brief Double-buffering frame buffer manager
 * 
 * Features:
 * - Double-buffering (front/back buffers)
 * - Dirty region tracking
 * - Partial screen updates
 * - PSRAM allocation for large buffers
 * - Circular display optimization
 */
class FrameBuffer {
public:
    /**
     * @brief Constructor
     * @param tft Pointer to TFT_eSPI instance
     */
    FrameBuffer(TFT_eSPI* tft);

    /**
     * @brief Destructor
     */
    ~FrameBuffer();

    /**
     * @brief Initialize frame buffers
     * @return true if successful
     */
    bool init();

    /**
     * @brief Begin frame rendering (switch to back buffer)
     */
    void beginFrame();

    /**
     * @brief End frame rendering and swap buffers
     * @param fullUpdate If true, update entire display
     */
    void endFrame(bool fullUpdate = false);

    /**
     * @brief Get current drawing sprite (back buffer)
     * @return Pointer to sprite
     */
    TFT_eSprite* getSprite() { return m_backBuffer; }

    /**
     * @brief Clear back buffer
     * @param color Fill color
     */
    void clear(uint16_t color = 0x0000);

    /**
     * @brief Mark region as dirty (needs redraw)
     * @param x X coordinate
     * @param y Y coordinate
     * @param width Width
     * @param height Height
     */
    void markDirty(int16_t x, int16_t y, int16_t width, int16_t height);

    /**
     * @brief Mark entire screen as dirty
     */
    void markAllDirty();

    /**
     * @brief Check if region is dirty
     * @param x X coordinate
     * @param y Y coordinate
     * @param width Width
     * @param height Height
     * @return true if any part is dirty
     */
    bool isDirty(int16_t x, int16_t y, int16_t width, int16_t height);

    /**
     * @brief Clear dirty flags
     */
    void clearDirtyFlags();

    /**
     * @brief Enable/disable dirty region tracking
     * @param enabled true to enable
     */
    void setDirtyTracking(bool enabled) { m_dirtyTracking = enabled; }

    /**
     * @brief Get frame count
     * @return Number of frames rendered
     */
    uint32_t getFrameCount() const { return m_frameCount; }

    /**
     * @brief Get last frame time in ms
     * @return Frame time
     */
    uint32_t getFrameTime() const { return m_frameTime; }

    /**
     * @brief Get FPS (frames per second)
     * @return Current FPS
     */
    float getFPS() const;

private:
    void swapBuffers();
    void updateDirtyRegions();

    TFT_eSPI* m_tft;
    TFT_eSprite* m_frontBuffer;
    TFT_eSprite* m_backBuffer;
    
    DirtyRegion m_dirtyRegion;
    bool m_dirtyTracking;
    
    uint32_t m_frameCount;
    uint32_t m_frameTime;
    uint32_t m_lastFrameTime;
    
    bool m_initialized;
};

#endif // FRAME_BUFFER_H


