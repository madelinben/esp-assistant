/**
 * @file HexagonalGrid.h
 * @brief Hexagonal grid layout (Apple Watch style) - MVC View Layer
 * 
 * Implements an interlocking hexagonal grid pattern for displaying
 * circular icons. Items spiral outward from center.
 * Part of MVC architecture - View layer (layout).
 */

#ifndef HEXAGONAL_GRID_H
#define HEXAGONAL_GRID_H

#include <Arduino.h>
#include <vector>
#include "config/Config.h"

/**
 * @struct GridItem
 * @brief Item in hexagonal grid
 */
struct GridItem {
    const char* label;
    const uint16_t* icon;     // Icon image data (RGB565)
    uint16_t backgroundColor;
    void (*onTap)();          // Callback when tapped
    void* userData;           // Custom data pointer
    
    // Calculated position
    int16_t x;
    int16_t y;
    int index;
    bool visible;
};

/**
 * @class HexagonalGrid
 * @brief Hexagonal grid layout manager (Apple Watch style)
 * 
 * Manages circular items in a hexagonal pattern:
 * - Center item first
 * - Spiral arrangement outward
 * - Drag to scroll
 * - Touch to select
 */
class HexagonalGrid {
public:
    /**
     * @brief Constructor
     * @param centerX Grid center X coordinate
     * @param centerY Grid center Y coordinate
     * @param itemRadius Radius of each circular item
     * @param spacing Spacing between items
     */
    HexagonalGrid(int16_t centerX = SCREEN_CENTER_X, 
                  int16_t centerY = SCREEN_CENTER_Y,
                  int16_t itemRadius = HEX_ITEM_RADIUS,
                  int16_t spacing = HEX_SPACING);

    ~HexagonalGrid();

    /**
     * @brief Add item to grid
     * @param item Item to add (will be copied)
     */
    void addItem(const GridItem& item);

    /**
     * @brief Remove item by index
     * @param index Item index
     */
    void removeItem(int index);

    /**
     * @brief Clear all items
     */
    void clear();

    /**
     * @brief Render grid to display
     */
    void render();

    /**
     * @brief Handle touch tap
     * @param x Touch X coordinate
     * @param y Touch Y coordinate
     * @return true if item was tapped
     */
    bool handleTap(int16_t x, int16_t y);

    /**
     * @brief Handle drag (scroll)
     * @param deltaX X drag amount
     * @param deltaY Y drag amount
     */
    void handleDrag(int16_t deltaX, int16_t deltaY);

    /**
     * @brief Get item at position
     * @param x X coordinate
     * @param y Y coordinate
     * @return Pointer to item or nullptr
     */
    GridItem* getItemAtPosition(int16_t x, int16_t y);

    /**
     * @brief Set scroll offset
     * @param x X offset
     * @param y Y offset
     */
    void setScrollOffset(int16_t x, int16_t y);

    /**
     * @brief Get scroll offset X
     */
    int16_t getScrollX() const { return m_scrollOffsetX; }

    /**
     * @brief Get scroll offset Y
     */
    int16_t getScrollY() const { return m_scrollOffsetY; }

    /**
     * @brief Get number of items
     */
    size_t getItemCount() const { return m_items.size(); }

    /**
     * @brief Get item by index
     * @param index Item index
     * @return Pointer to item or nullptr
     */
    GridItem* getItem(int index);

    /**
     * @brief Enable/disable smooth scrolling animation
     */
    void setSmoothScrolling(bool enabled) { m_smoothScrolling = enabled; }

private:
    void calculateItemPositions();
    void getHexPosition(int index, int16_t& x, int16_t& y);
    void drawItem(const GridItem& item);
    bool isItemVisible(const GridItem& item);
    void updateScrollBounds();

    std::vector<GridItem> m_items;
    int16_t m_centerX;
    int16_t m_centerY;
    int16_t m_itemRadius;
    int16_t m_spacing;
    int16_t m_scrollOffsetX;
    int16_t m_scrollOffsetY;
    int16_t m_maxScrollX;
    int16_t m_maxScrollY;
    
    // Hexagonal pattern parameters
    float m_hexDistance;  // Distance between hex centers
    bool m_smoothScrolling;
    
    // Hexagonal spiral pattern (ring indices)
    // Ring 0: 1 item (center)
    // Ring 1: 6 items
    // Ring 2: 12 items
    // Ring 3: 18 items
    static const int RING_ITEMS[];
};

#endif // HEXAGONAL_GRID_H


