/**
 * @file SpotifyView.cpp
 * @brief Implementation of SpotifyView
 */

#include "views/apps/spotify/SpotifyView.h"
#include "hardware/display/DisplayDriver.h"

SpotifyView::SpotifyView()
    : m_controller(nullptr)
    , m_volumeSlider(nullptr)
    , m_seekSlider(nullptr)
    , m_currentTab(SpotifyTab::PLAYBACK)
    , m_isDragging(false)
    , m_lastUpdate(0)
    , m_updateInterval(1000) {
    
    m_isActive = false;
    m_lastTouch = {0, 0, false, 0};
}

SpotifyView::~SpotifyView() {
    if (m_volumeSlider) {
        delete m_volumeSlider;
    }
    if (m_seekSlider) {
        delete m_seekSlider;
    }
}

void SpotifyView::onEnter() {
    DEBUG_PRINTLN("[SpotifyView] Entering...");
    
    m_isActive = true;
    m_currentTab = SpotifyTab::PLAYBACK;
    m_controller = &SpotifyController::getInstance();

    // Initialize controller if needed
    if (!m_controller->isAuthenticated()) {
        DEBUG_PRINTLN("[SpotifyView] WARNING: Spotify not authenticated!");
        // TODO: Show authentication required message
    }

    // Create sliders
    if (!m_volumeSlider) {
        m_volumeSlider = new CircularSlider(SCREEN_CENTER_X, SCREEN_CENTER_Y, 120);
        m_volumeSlider->setColors(TFT_DARKGREY, TFT_GREEN, TFT_WHITE);
    }

    if (!m_seekSlider) {
        m_seekSlider = new CircularSlider(SCREEN_CENTER_X, SCREEN_CENTER_Y, 120);
        m_seekSlider->setColors(TFT_DARKGREY, TFT_BLUE, TFT_WHITE);
    }

    // Initial update
    updateNowPlaying();

    DEBUG_PRINTLN("[SpotifyView] Entered");
}

void SpotifyView::onExit() {
    DEBUG_PRINTLN("[SpotifyView] Exiting...");
    m_isActive = false;
}

void SpotifyView::update() {
    if (!m_controller || !m_controller->isAuthenticated()) {
        return;
    }

    // Update now playing periodically
    uint32_t currentTime = millis();
    if (currentTime - m_lastUpdate >= m_updateInterval) {
        updateNowPlaying();
        m_lastUpdate = currentTime;
    }

    // Update seek slider position if playing
    SpotifyTrack* track = m_controller->getCurrentTrack();
    if (track && track->isPlaying()) {
        // Estimate position (update from server less frequently)
        int estimatedPosition = track->getPosition() + (currentTime - m_lastUpdate);
        if (m_seekSlider) {
            float progress = (float)estimatedPosition / (float)track->getDuration();
            m_seekSlider->setValue(progress);
        }
    }
}

void SpotifyView::render() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // Clear background
    sprite->fillSprite(TFT_BLACK);
    
    // TODO: Draw gradient background based on album art colors

    // Draw circular border
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y, SCREEN_RADIUS, TFT_BLUE);

    // Render album art (center)
    renderAlbumArt();

    // Render track info
    renderTrackInfo();

    // Render current tab
    switch (m_currentTab) {
        case SpotifyTab::PLAYBACK:
            renderPlaybackControls();
            break;
        case SpotifyTab::VOLUME:
            renderVolumeSlider();
            break;
        case SpotifyTab::SEEK:
            renderSeekSlider();
            break;
    }

    // Draw tab indicators at bottom
    int16_t tabY = SCREEN_HEIGHT - 30;
    sprite->setTextColor(m_currentTab == SpotifyTab::PLAYBACK ? TFT_WHITE : TFT_DARKGREY);
    sprite->setTextDatum(BC_DATUM);
    sprite->drawString("PLAY", SCREEN_CENTER_X - 60, tabY);
    
    sprite->setTextColor(m_currentTab == SpotifyTab::VOLUME ? TFT_WHITE : TFT_DARKGREY);
    sprite->drawString("VOL", SCREEN_CENTER_X, tabY);
    
    sprite->setTextColor(m_currentTab == SpotifyTab::SEEK ? TFT_WHITE : TFT_DARKGREY);
    sprite->drawString("SEEK", SCREEN_CENTER_X + 60, tabY);
}

void SpotifyView::renderAlbumArt() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    // TODO: Download and display actual album art
    // For now, draw a placeholder circle
    sprite->fillCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y - 20, 60, TFT_DARKGREY);
    sprite->drawCircle(SCREEN_CENTER_X, SCREEN_CENTER_Y - 20, 60, TFT_WHITE);
    
    // Draw play/pause icon
    SpotifyTrack* track = m_controller ? m_controller->getCurrentTrack() : nullptr;
    if (track && track->isPlaying()) {
        // Pause icon (two bars)
        sprite->fillRect(SCREEN_CENTER_X - 15, SCREEN_CENTER_Y - 30, 10, 20, TFT_WHITE);
        sprite->fillRect(SCREEN_CENTER_X + 5, SCREEN_CENTER_Y - 30, 10, 20, TFT_WHITE);
    } else {
        // Play icon (triangle)
        sprite->fillTriangle(SCREEN_CENTER_X - 10, SCREEN_CENTER_Y - 30,
                            SCREEN_CENTER_X - 10, SCREEN_CENTER_Y - 10,
                            SCREEN_CENTER_X + 10, SCREEN_CENTER_Y - 20,
                            TFT_WHITE);
    }
}

void SpotifyView::renderTrackInfo() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite || !m_controller) return;

    SpotifyTrack* track = m_controller->getCurrentTrack();
    if (!track || !track->isValid()) {
        sprite->setTextColor(TFT_DARKGREY);
        sprite->setTextDatum(MC_DATUM);
        sprite->drawString("No track playing", SCREEN_CENTER_X, SCREEN_CENTER_Y + 60);
        return;
    }

    // Draw track name
    sprite->setTextColor(TFT_WHITE);
    sprite->setTextDatum(TC_DATUM);
    sprite->drawString(track->getName(), SCREEN_CENTER_X, SCREEN_CENTER_Y + 50);

    // Draw artist
    sprite->setTextColor(TFT_LIGHTGREY);
    sprite->drawString(track->getArtist(), SCREEN_CENTER_X, SCREEN_CENTER_Y + 70);

    // Draw album (smaller)
    sprite->setTextColor(TFT_DARKGREY);
    sprite->drawString(track->getAlbum(), SCREEN_CENTER_X, SCREEN_CENTER_Y + 90);
}

void SpotifyView::renderPlaybackControls() {
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    
    if (!sprite) return;

    int16_t controlY = SCREEN_HEIGHT - 80;
    int16_t spacing = 60;

    // Previous button
    sprite->fillTriangle(SCREEN_CENTER_X - spacing - 10, controlY - 10,
                        SCREEN_CENTER_X - spacing - 10, controlY + 10,
                        SCREEN_CENTER_X - spacing - 20, controlY,
                        TFT_WHITE);

    // Play/Pause button (larger)
    SpotifyTrack* track = m_controller ? m_controller->getCurrentTrack() : nullptr;
    if (track && track->isPlaying()) {
        // Pause
        sprite->fillRect(SCREEN_CENTER_X - 12, controlY - 15, 8, 30, TFT_WHITE);
        sprite->fillRect(SCREEN_CENTER_X + 4, controlY - 15, 8, 30, TFT_WHITE);
    } else {
        // Play
        sprite->fillTriangle(SCREEN_CENTER_X - 10, controlY - 15,
                            SCREEN_CENTER_X - 10, controlY + 15,
                            SCREEN_CENTER_X + 15, controlY,
                            TFT_WHITE);
    }

    // Next button
    sprite->fillTriangle(SCREEN_CENTER_X + spacing + 10, controlY - 10,
                        SCREEN_CENTER_X + spacing + 10, controlY + 10,
                        SCREEN_CENTER_X + spacing + 20, controlY,
                        TFT_WHITE);
}

void SpotifyView::renderVolumeSlider() {
    if (!m_volumeSlider || !m_controller) return;

    // Update slider value from track
    SpotifyTrack* track = m_controller->getCurrentTrack();
    if (track) {
        m_volumeSlider->setValue(track->getVolume() / 100.0f);
    }

    // Render slider
    m_volumeSlider->render();

    // Draw volume percentage
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    if (sprite && track) {
        char volStr[8];
        snprintf(volStr, sizeof(volStr), "%d%%", track->getVolume());
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextDatum(MC_DATUM);
        sprite->drawString(volStr, SCREEN_CENTER_X, SCREEN_CENTER_Y - 20);
    }
}

void SpotifyView::renderSeekSlider() {
    if (!m_seekSlider || !m_controller) return;

    SpotifyTrack* track = m_controller->getCurrentTrack();
    if (!track || !track->isValid()) return;

    // Update slider value
    float progress = (float)track->getPosition() / (float)track->getDuration();
    m_seekSlider->setValue(progress);

    // Render slider
    m_seekSlider->render();

    // Draw time info
    DisplayDriver& display = DisplayDriver::getInstance();
    TFT_eSprite* sprite = display.getSprite();
    if (sprite) {
        String timeStr = track->formatPosition() + " / " + track->formatDuration();
        sprite->setTextColor(TFT_WHITE);
        sprite->setTextDatum(MC_DATUM);
        sprite->drawString(timeStr, SCREEN_CENTER_X, SCREEN_CENTER_Y - 20);
    }
}

void SpotifyView::updateNowPlaying() {
    if (!m_controller || !m_controller->isAuthenticated()) {
        return;
    }

    DEBUG_PRINTLN("[SpotifyView] Updating now playing...");
    m_controller->updateNowPlaying();
}

void SpotifyView::handleTouch(TouchEvent event) {
    TouchController& touchCtrl = TouchController::getInstance();
    TouchPoint currentTouch = touchCtrl.getCurrentTouch();

    switch (event) {
        case TouchEvent::TAP:
            if (m_currentTab == SpotifyTab::PLAYBACK) {
                handlePlaybackTouch(currentTouch.x, currentTouch.y);
            }
            break;

        case TouchEvent::DRAG_MOVE:
            if (m_currentTab == SpotifyTab::VOLUME && m_volumeSlider) {
                if (m_volumeSlider->handleDrag(currentTouch.x, currentTouch.y)) {
                    // Update volume on Spotify
                    int volume = (int)(m_volumeSlider->getValue() * 100);
                    m_controller->setVolume(volume);
                }
            } else if (m_currentTab == SpotifyTab::SEEK && m_seekSlider) {
                if (m_seekSlider->handleDrag(currentTouch.x, currentTouch.y)) {
                    // Update seek position on Spotify
                    SpotifyTrack* track = m_controller->getCurrentTrack();
                    if (track) {
                        int position = (int)(m_seekSlider->getValue() * track->getDuration());
                        m_controller->seek(position);
                    }
                }
            }
            break;

        case TouchEvent::SWIPE_LEFT:
            // Next tab
            if (m_currentTab == SpotifyTab::PLAYBACK) {
                m_currentTab = SpotifyTab::VOLUME;
            } else if (m_currentTab == SpotifyTab::VOLUME) {
                m_currentTab = SpotifyTab::SEEK;
            }
            break;

        case TouchEvent::SWIPE_RIGHT:
            // Previous tab
            if (m_currentTab == SpotifyTab::SEEK) {
                m_currentTab = SpotifyTab::VOLUME;
            } else if (m_currentTab == SpotifyTab::VOLUME) {
                m_currentTab = SpotifyTab::PLAYBACK;
            }
            break;

        default:
            break;
    }

    m_lastTouch = currentTouch;
}

void SpotifyView::handlePlaybackTouch(int16_t x, int16_t y) {
    int16_t controlY = SCREEN_HEIGHT - 80;
    int16_t spacing = 60;

    // Check which button was tapped
    // Previous button
    if (x >= SCREEN_CENTER_X - spacing - 30 && x <= SCREEN_CENTER_X - spacing + 10 &&
        y >= controlY - 20 && y <= controlY + 20) {
        DEBUG_PRINTLN("[SpotifyView] Previous tapped");
        m_controller->skipPrevious();
        return;
    }

    // Play/Pause button
    if (x >= SCREEN_CENTER_X - 30 && x <= SCREEN_CENTER_X + 30 &&
        y >= controlY - 20 && y <= controlY + 20) {
        DEBUG_PRINTLN("[SpotifyView] Play/Pause tapped");
        m_controller->togglePlayPause();
        return;
    }

    // Next button
    if (x >= SCREEN_CENTER_X + spacing - 10 && x <= SCREEN_CENTER_X + spacing + 30 &&
        y >= controlY - 20 && y <= controlY + 20) {
        DEBUG_PRINTLN("[SpotifyView] Next tapped");
        m_controller->skipNext();
        return;
    }
}

PageView* createSpotifyView() {
    return new SpotifyView();
}


