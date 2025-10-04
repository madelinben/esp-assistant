/**
 * @file HexagonalGrid.cpp
 * @brief Implementation of HexagonalGrid
 */

#include "views/layouts/HexagonalGrid.h"
#include "hardware/display/DisplayDriver.h"
#include <cmath>

// Hexagonal spiral pattern
const int HexagonalGrid::RING_ITEMS[] = {1, 6, 12, 18, 24, 30};

HexagonalGrid::HexagonalGrid(int16_t centerX, int16_t centerY, int16_t itemRadius, int16_t spacing)
    : m_centerX(centerX)
    , m_centerY(centerY)
    , m_itemRadius(itemRadius)
    , m_spacing(spacing)
    , m_scrollOffsetX(0)
    , m_scrollOffsetY(0)
    , m_maxScrollX(0)
    , m_maxScrollY(0)
    , m_smoothScrolling(true) {
    
    // Calculate hexagonal distance
    // For a perfect hexagonal pattern, distance = itemDiameter + spacing
    m_hexDistance = (itemRadius * 2) + spacing;
}

HexagonalGrid::~HexagonalGrid() {
    clear();
}

void HexagonalGrid::addItem(const GridItem& item) {
    GridItem newItem = item;
    newItem.index = m_items.size();
    newItem.visible = false;
    m_items.push_back(newItem);
    
    calculateItemPositions();
    updateScrollBounds();
}

void HexagonalGrid::removeItem(int index) {
    if (index >= 0 && index < m_items.size()) {
        m_items.erase(m_items.begin() + index);
        
        // Reindex remaining items
        for (int i = 0; i < m_items.size(); i++) {
            m_items[i].index = i;
        }
        
        calculateItemPositions();
        updateScrollBounds();
    }
}

void HexagonalGrid::clear() {
    m_items.clear();
    m_scrollOffsetX = 0;
    m_scrollOffsetY = 0;
}

void HexagonalGrid::calculateItemPositions() {
    for (int i = 0; i < m_items.size(); i++) {
        getHexPosition(i, m_items[i].x, m_items[i].y);
    }
}

void HexagonalGrid::getHexPosition(int index, int16_t& x, int16_t& y) {
    if (index == 0) {
        // Center item
        x = m_centerX;
        y = m_centerY;
        return;
    }

    // Calculate which ring this index belongs to
    int ring = 0;
    int itemsBeforeRing = 0;
    int cumulativeItems = 0;
    
    for (int r = 0; r < 6; r++) {
        cumulativeItems += RING_ITEMS[r];
        if (index < cumulativeItems) {
            ring = r;
            break;
        }
        itemsBeforeRing = cumulativeItems;
    }

    // Position within ring
    int positionInRing = index - itemsBeforeRing;
    int itemsInRing = RING_ITEMS[ring];
    
    // Calculate angle for this position
    // Hexagonal arrangement: divide circle into itemsInRing segments
    float angle = (2.0f * PI * positionInRing) / itemsInRing;
    
    // Add small rotation offset for aesthetics
    angle += PI / 6.0f;
    
    // Calculate position
    float radius = ring * m_hexDistance;
    x = m_centerX + (int16_t)(radius * cos(angle));
    y = m_centerY + (int16_t)(radius * sin(angle));
}

void HexagonalGrid::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Draw items
    for (int i = 0; i < m_items.size(); i++) {
        m_items[i].visible = isItemVisible(m_items[i]);
        
        if (m_items[i].visible) {
            drawItem(m_items[i]);
        }
    }
}

void HexagonalGrid::drawItem(const GridItem& item) {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Calculate screen position with scroll offset
    int16_t screenX = item.x + m_scrollOffsetX;
    int16_t screenY = item.y + m_scrollOffsetY;

    // Draw background circle
    sprite->fillCircle(screenX, screenY, m_itemRadius, item.backgroundColor);
    
    // Draw border
    sprite->drawCircle(screenX, screenY, m_itemRadius, TFT_WHITE);

    // Draw icon if available
    if (item.icon) {
        // TODO: Draw icon image
        // For now, draw a simple indicator
        sprite->fillCircle(screenX, screenY, m_itemRadius / 2, TFT_WHITE);
    }

    // Draw label below icon
    if (item.label) {
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextDatum(TC_DATUM);  // Top center
        sprite->drawString(item.label, screenX, screenY + m_itemRadius + 5);
    }
}

bool HexagonalGrid::isItemVisible(const GridItem& item) {
    int16_t screenX = item.x + m_scrollOffsetX;
    int16_t screenY = item.y + m_scrollOffsetY;
    
    // Check if item is within screen bounds (with margin)
    int16_t margin = m_itemRadius + 10;
    
    return (screenX >= -margin && screenX < SCREEN_WIDTH + margin &&
            screenY >= -margin && screenY < SCREEN_HEIGHT + margin);
}

bool HexagonalGrid::handleTap(int16_t x, int16_t y) {
    GridItem* item = getItemAtPosition(x, y);
    
    if (item && item->onTap) {
        item->onTap();
        return true;
    }
    
    return false;
}

void HexagonalGrid::handleDrag(int16_t deltaX, int16_t deltaY) {
    m_scrollOffsetX += deltaX;
    m_scrollOffsetY += deltaY;
    
    // Clamp scroll to bounds
    if (m_scrollOffsetX > m_maxScrollX) m_scrollOffsetX = m_maxScrollX;
    if (m_scrollOffsetX < -m_maxScrollX) m_scrollOffsetX = -m_maxScrollX;
    if (m_scrollOffsetY > m_maxScrollY) m_scrollOffsetY = m_maxScrollY;
    if (m_scrollOffsetY < -m_maxScrollY) m_scrollOffsetY = -m_maxScrollY;
}

GridItem* HexagonalGrid::getItemAtPosition(int16_t x, int16_t y) {
    for (int i = 0; i < m_items.size(); i++) {
        if (!m_items[i].visible) continue;
        
        int16_t screenX = m_items[i].x + m_scrollOffsetX;
        int16_t screenY = m_items[i].y + m_scrollOffsetY;
        
        // Check if touch is within item circle
        int16_t dx = x - screenX;
        int16_t dy = y - screenY;
        int16_t distSquared = dx * dx + dy * dy;
        
        if (distSquared <= m_itemRadius * m_itemRadius) {
            return &m_items[i];
        }
    }
    
    return nullptr;
}

void HexagonalGrid::setScrollOffset(int16_t x, int16_t y) {
    m_scrollOffsetX = x;
    m_scrollOffsetY = y;
    
    // Clamp to bounds
    if (m_scrollOffsetX > m_maxScrollX) m_scrollOffsetX = m_maxScrollX;
    if (m_scrollOffsetX < -m_maxScrollX) m_scrollOffsetX = -m_maxScrollX;
    if (m_scrollOffsetY > m_maxScrollY) m_scrollOffsetY = m_maxScrollY;
    if (m_scrollOffsetY < -m_maxScrollY) m_scrollOffsetY = -m_maxScrollY;
}

GridItem* HexagonalGrid::getItem(int index) {
    if (index >= 0 && index < m_items.size()) {
        return &m_items[index];
    }
    return nullptr;
}

void HexagonalGrid::updateScrollBounds() {
    if (m_items.empty()) {
        m_maxScrollX = 0;
        m_maxScrollY = 0;
        return;
    }

    // Find extents of grid
    int16_t minX = m_items[0].x;
    int16_t maxX = m_items[0].x;
    int16_t minY = m_items[0].y;
    int16_t maxY = m_items[0].y;
    
    for (const auto& item : m_items) {
        if (item.x < minX) minX = item.x;
        if (item.x > maxX) maxX = item.x;
        if (item.y < minY) minY = item.y;
        if (item.y > maxY) maxY = item.y;
    }

    // Calculate max scroll needed to keep grid visible
    int16_t gridWidth = maxX - minX + (m_itemRadius * 2);
    int16_t gridHeight = maxY - minY + (m_itemRadius * 2);
    
    m_maxScrollX = max(0, (gridWidth - SCREEN_WIDTH) / 2);
    m_maxScrollY = max(0, (gridHeight - SCREEN_HEIGHT) / 2);
}


