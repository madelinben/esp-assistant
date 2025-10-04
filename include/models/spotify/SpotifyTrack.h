/**
 * @file SpotifyTrack.h
 * @brief Spotify track model - MVC Model Layer
 * 
 * Represents Spotify track data and playback state.
 * Part of MVC architecture - Model layer.
 */

#ifndef SPOTIFY_TRACK_H
#define SPOTIFY_TRACK_H

#include <Arduino.h>

/**
 * @enum PlaybackState
 * @brief Playback state
 */
enum class PlaybackState {
    STOPPED,
    PLAYING,
    PAUSED,
    BUFFERING
};

/**
 * @enum RepeatMode
 * @brief Repeat mode
 */
enum class RepeatMode {
    OFF,
    TRACK,
    CONTEXT
};

/**
 * @class SpotifyTrack
 * @brief Model representing a Spotify track
 * 
 * Contains track metadata and playback information
 */
class SpotifyTrack {
public:
    /**
     * @brief Constructor
     */
    SpotifyTrack();

    /**
     * @brief Constructor with data
     */
    SpotifyTrack(const String& id, const String& name, const String& artist, 
                 const String& album, int duration);

    // Getters
    String getId() const { return m_id; }
    String getName() const { return m_name; }
    String getArtist() const { return m_artist; }
    String getAlbum() const { return m_album; }
    String getAlbumArtUrl() const { return m_albumArtUrl; }
    int getDuration() const { return m_duration; }  // milliseconds
    int getPosition() const { return m_position; }  // milliseconds
    PlaybackState getPlaybackState() const { return m_playbackState; }
    int getVolume() const { return m_volume; }  // 0-100
    bool isShuffle() const { return m_shuffle; }
    RepeatMode getRepeatMode() const { return m_repeatMode; }
    String getContextUri() const { return m_contextUri; }  // Playlist/album URI

    // Setters
    void setId(const String& id) { m_id = id; }
    void setName(const String& name) { m_name = name; }
    void setArtist(const String& artist) { m_artist = artist; }
    void setAlbum(const String& album) { m_album = album; }
    void setAlbumArtUrl(const String& url) { m_albumArtUrl = url; }
    void setDuration(int duration) { m_duration = duration; }
    void setPosition(int position) { m_position = position; }
    void setPlaybackState(PlaybackState state) { m_playbackState = state; }
    void setVolume(int volume);  // Clamps to 0-100
    void setShuffle(bool shuffle) { m_shuffle = shuffle; }
    void setRepeatMode(RepeatMode mode) { m_repeatMode = mode; }
    void setContextUri(const String& uri) { m_contextUri = uri; }

    // Utility methods
    /**
     * @brief Get progress as percentage (0-100)
     */
    int getProgressPercent() const;

    /**
     * @brief Format duration as MM:SS
     */
    String formatDuration() const;

    /**
     * @brief Format position as MM:SS
     */
    String formatPosition() const;

    /**
     * @brief Get remaining time in milliseconds
     */
    int getTimeRemaining() const;

    /**
     * @brief Check if track is playing
     */
    bool isPlaying() const { return m_playbackState == PlaybackState::PLAYING; }

    /**
     * @brief Check if track is paused
     */
    bool isPaused() const { return m_playbackState == PlaybackState::PAUSED; }

    /**
     * @brief Check if track data is valid
     */
    bool isValid() const;

    /**
     * @brief Clear track data
     */
    void clear();

private:
    String formatTime(int milliseconds) const;

    String m_id;
    String m_name;
    String m_artist;
    String m_album;
    String m_albumArtUrl;
    int m_duration;          // Total duration in ms
    int m_position;          // Current position in ms
    PlaybackState m_playbackState;
    int m_volume;            // 0-100
    bool m_shuffle;
    RepeatMode m_repeatMode;
    String m_contextUri;     // Playlist/album context
};

#endif // SPOTIFY_TRACK_H


