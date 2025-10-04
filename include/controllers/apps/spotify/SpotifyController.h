/**
 * @file SpotifyController.h
 * @brief Spotify controller with Web API - MVC Controller Layer
 * 
 * Controller for Spotify Web API integration and remote playback control.
 * Part of MVC architecture - Controller layer (app).
 */

#ifndef SPOTIFY_CONTROLLER_H
#define SPOTIFY_CONTROLLER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "models/spotify/SpotifyTrack.h"
#include "services/DatabaseService.h"
#include "services/AuthService.h"

/**
 * @class SpotifyController
 * @brief Controller for Spotify Web API
 * 
 * Features:
 * - OAuth authentication
 * - Now playing updates
 * - Playback control (play/pause/skip)
 * - Volume control
 * - Seek control
 * - Playlist/album context
 */
class SpotifyController {
public:
    /**
     * @brief Get singleton instance
     */
    static SpotifyController& getInstance();

    /**
     * @brief Initialize Spotify controller
     * @return true if successful
     */
    bool init();

    /**
     * @brief Set access token
     * @param token OAuth access token
     */
    void setAccessToken(const String& token);

    /**
     * @brief Get current access token
     */
    String getAccessToken() const { return m_accessToken; }

    /**
     * @brief Update now playing information
     * @return true if successful
     */
    bool updateNowPlaying();

    /**
     * @brief Get current track
     */
    SpotifyTrack* getCurrentTrack() { return &m_currentTrack; }

    /**
     * @brief Play/resume playback
     * @return true if successful
     */
    bool play();

    /**
     * @brief Pause playback
     * @return true if successful
     */
    bool pause();

    /**
     * @brief Toggle play/pause
     * @return true if successful
     */
    bool togglePlayPause();

    /**
     * @brief Skip to next track
     * @return true if successful
     */
    bool skipNext();

    /**
     * @brief Skip to previous track
     * @return true if successful
     */
    bool skipPrevious();

    /**
     * @brief Set volume
     * @param volume Volume level (0-100)
     * @return true if successful
     */
    bool setVolume(int volume);

    /**
     * @brief Seek to position
     * @param position Position in milliseconds
     * @return true if successful
     */
    bool seek(int position);

    /**
     * @brief Set shuffle mode
     * @param shuffle true to enable shuffle
     * @return true if successful
     */
    bool setShuffle(bool shuffle);

    /**
     * @brief Set repeat mode
     * @param mode Repeat mode
     * @return true if successful
     */
    bool setRepeat(RepeatMode mode);

    /**
     * @brief Check if authenticated
     */
    bool isAuthenticated() const { return m_accessToken.length() > 0; }

    /**
     * @brief Get last error message
     */
    String getLastError() const { return m_lastError; }

private:
    SpotifyController();
    ~SpotifyController();
    SpotifyController(const SpotifyController&) = delete;
    SpotifyController& operator=(const SpotifyController&) = delete;

    bool makeApiRequest(const String& endpoint, const String& method, 
                       const String& body, String& response);
    bool parseNowPlaying(const String& json);
    void loadAccessToken();
    void saveAccessToken();

    String m_accessToken;
    SpotifyTrack m_currentTrack;
    String m_lastError;
    HTTPClient m_http;
    bool m_initialized;

    // Spotify Web API endpoints
    static constexpr const char* API_BASE = "https://api.spotify.com/v1";
    static constexpr const char* EP_NOW_PLAYING = "/me/player/currently-playing";
    static constexpr const char* EP_PLAY = "/me/player/play";
    static constexpr const char* EP_PAUSE = "/me/player/pause";
    static constexpr const char* EP_NEXT = "/me/player/next";
    static constexpr const char* EP_PREVIOUS = "/me/player/previous";
    static constexpr const char* EP_VOLUME = "/me/player/volume";
    static constexpr const char* EP_SEEK = "/me/player/seek";
    static constexpr const char* EP_SHUFFLE = "/me/player/shuffle";
    static constexpr const char* EP_REPEAT = "/me/player/repeat";
};

#endif // SPOTIFY_CONTROLLER_H


