/**
 * @file SpotifyTrack.cpp
 * @brief Implementation of SpotifyTrack
 */

#include "models/spotify/SpotifyTrack.h"

SpotifyTrack::SpotifyTrack()
    : m_id("")
    , m_name("")
    , m_artist("")
    , m_album("")
    , m_albumArtUrl("")
    , m_duration(0)
    , m_position(0)
    , m_playbackState(PlaybackState::STOPPED)
    , m_volume(70)
    , m_shuffle(false)
    , m_repeatMode(RepeatMode::OFF)
    , m_contextUri("") {
}

SpotifyTrack::SpotifyTrack(const String& id, const String& name, const String& artist, 
                           const String& album, int duration)
    : m_id(id)
    , m_name(name)
    , m_artist(artist)
    , m_album(album)
    , m_albumArtUrl("")
    , m_duration(duration)
    , m_position(0)
    , m_playbackState(PlaybackState::STOPPED)
    , m_volume(70)
    , m_shuffle(false)
    , m_repeatMode(RepeatMode::OFF)
    , m_contextUri("") {
}

void SpotifyTrack::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    m_volume = volume;
}

int SpotifyTrack::getProgressPercent() const {
    if (m_duration <= 0) return 0;
    return (m_position * 100) / m_duration;
}

String SpotifyTrack::formatDuration() const {
    return formatTime(m_duration);
}

String SpotifyTrack::formatPosition() const {
    return formatTime(m_position);
}

String SpotifyTrack::formatTime(int milliseconds) const {
    int totalSeconds = milliseconds / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%d:%02d", minutes, seconds);
    return String(buffer);
}

int SpotifyTrack::getTimeRemaining() const {
    return m_duration - m_position;
}

bool SpotifyTrack::isValid() const {
    return m_id.length() > 0 && m_name.length() > 0;
}

void SpotifyTrack::clear() {
    m_id = "";
    m_name = "";
    m_artist = "";
    m_album = "";
    m_albumArtUrl = "";
    m_duration = 0;
    m_position = 0;
    m_playbackState = PlaybackState::STOPPED;
    m_volume = 70;
    m_shuffle = false;
    m_repeatMode = RepeatMode::OFF;
    m_contextUri = "";
}


