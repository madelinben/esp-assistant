/**
 * @file SpotifyView.h
 * @brief Spotify app view with playback controls - MVC View Layer
 * 
 * View for Spotify remote playback control with circular sliders.
 * Part of MVC architecture - View layer (app).
 */

#ifndef SPOTIFY_VIEW_H
#define SPOTIFY_VIEW_H

#include "controllers/NavigationController.h"
#include "controllers/apps/spotify/SpotifyController.h"
#include "views/components/CircularSlider.h"

/**
 * @enum SpotifyTab
 * @brief Spotify view tabs
 */
enum class SpotifyTab {
    PLAYBACK,    // Play/pause, next, previous
    VOLUME,      // Volume slider
    SEEK         // Seek slider
};

/**
 * @class SpotifyView
 * @brief Spotify app page
 * 
 * Features:
 * - Album art display (center)
 * - Gradient background from album colors
 * - Song title and artist
 * - Tabs: Playback controls, Volume slider, Seek slider
 * - Now playing updates
 */
class SpotifyView : public PageView {
public:
    /**
     * @brief Constructor
     */
    SpotifyView();

    /**
     * @brief Destructor
     */
    ~SpotifyView();

    // PageView interface
    void onEnter() override;
    void onExit() override;
    void update() override;
    void render() override;
    void handleTouch(TouchEvent event) override;
    const char* getName() const override { return "Spotify"; }

private:
    void renderAlbumArt();
    void renderTrackInfo();
    void renderPlaybackControls();
    void renderVolumeSlider();
    void renderSeekSlider();
    void updateNowPlaying();
    void handlePlaybackTouch(int16_t x, int16_t y);

    SpotifyController* m_controller;
    CircularSlider* m_volumeSlider;
    CircularSlider* m_seekSlider;
    
    SpotifyTab m_currentTab;
    TouchPoint m_lastTouch;
    bool m_isDragging;
    
    uint32_t m_lastUpdate;
    uint32_t m_updateInterval;  // ms
};

/**
 * @brief Factory function for SpotifyView
 * @return Pointer to new SpotifyView instance
 */
PageView* createSpotifyView();

#endif // SPOTIFY_VIEW_H


