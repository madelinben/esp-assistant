/**
 * @file SpotifyController.cpp
 * @brief Implementation of SpotifyController
 */

#include "controllers/apps/spotify/SpotifyController.h"
#include "config/Config.h"

SpotifyController& SpotifyController::getInstance() {
    static SpotifyController instance;
    return instance;
}

SpotifyController::SpotifyController()
    : m_accessToken("")
    , m_lastError("")
    , m_initialized(false) {
}

SpotifyController::~SpotifyController() {
}

bool SpotifyController::init() {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[SpotifyController] Initializing...");

    // Load access token from database
    loadAccessToken();

    if (!isAuthenticated()) {
        DEBUG_PRINTLN("[SpotifyController] WARNING: No access token found");
        DEBUG_PRINTLN("[SpotifyController] User needs to authenticate via Spotify OAuth");
        // Don't fail init - user can authenticate later
    }

    m_initialized = true;
    DEBUG_PRINTLN("[SpotifyController] Initialized");
    return true;
}

void SpotifyController::setAccessToken(const String& token) {
    m_accessToken = token;
    saveAccessToken();
    DEBUG_PRINTLN("[SpotifyController] Access token set");
}

bool SpotifyController::updateNowPlaying() {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    DEBUG_PRINTLN("[SpotifyController] Updating now playing...");

    String response;
    if (!makeApiRequest(EP_NOW_PLAYING, "GET", "", response)) {
        DEBUG_PRINTF("[SpotifyController] Failed to get now playing: %s\n", m_lastError.c_str());
        return false;
    }

    if (response.length() == 0) {
        DEBUG_PRINTLN("[SpotifyController] No track currently playing");
        m_currentTrack.clear();
        return true;
    }

    return parseNowPlaying(response);
}

bool SpotifyController::play() {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    DEBUG_PRINTLN("[SpotifyController] Play");

    String response;
    if (makeApiRequest(EP_PLAY, "PUT", "", response)) {
        m_currentTrack.setPlaybackState(PlaybackState::PLAYING);
        return true;
    }

    return false;
}

bool SpotifyController::pause() {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    DEBUG_PRINTLN("[SpotifyController] Pause");

    String response;
    if (makeApiRequest(EP_PAUSE, "PUT", "", response)) {
        m_currentTrack.setPlaybackState(PlaybackState::PAUSED);
        return true;
    }

    return false;
}

bool SpotifyController::togglePlayPause() {
    if (m_currentTrack.isPlaying()) {
        return pause();
    } else {
        return play();
    }
}

bool SpotifyController::skipNext() {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    DEBUG_PRINTLN("[SpotifyController] Skip next");

    String response;
    if (makeApiRequest(EP_NEXT, "POST", "", response)) {
        // Update now playing after skip
        delay(500);  // Wait for Spotify to update
        return updateNowPlaying();
    }

    return false;
}

bool SpotifyController::skipPrevious() {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    DEBUG_PRINTLN("[SpotifyController] Skip previous");

    String response;
    if (makeApiRequest(EP_PREVIOUS, "POST", "", response)) {
        // Update now playing after skip
        delay(500);
        return updateNowPlaying();
    }

    return false;
}

bool SpotifyController::setVolume(int volume) {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;

    DEBUG_PRINTF("[SpotifyController] Set volume: %d%%\n", volume);

    String endpoint = String(EP_VOLUME) + "?volume_percent=" + String(volume);
    String response;
    
    if (makeApiRequest(endpoint.c_str(), "PUT", "", response)) {
        m_currentTrack.setVolume(volume);
        return true;
    }

    return false;
}

bool SpotifyController::seek(int position) {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    DEBUG_PRINTF("[SpotifyController] Seek to: %d ms\n", position);

    String endpoint = String(EP_SEEK) + "?position_ms=" + String(position);
    String response;
    
    if (makeApiRequest(endpoint.c_str(), "PUT", "", response)) {
        m_currentTrack.setPosition(position);
        return true;
    }

    return false;
}

bool SpotifyController::setShuffle(bool shuffle) {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    DEBUG_PRINTF("[SpotifyController] Set shuffle: %s\n", shuffle ? "ON" : "OFF");

    String endpoint = String(EP_SHUFFLE) + "?state=" + String(shuffle ? "true" : "false");
    String response;
    
    if (makeApiRequest(endpoint.c_str(), "PUT", "", response)) {
        m_currentTrack.setShuffle(shuffle);
        return true;
    }

    return false;
}

bool SpotifyController::setRepeat(RepeatMode mode) {
    if (!isAuthenticated()) {
        m_lastError = "Not authenticated";
        return false;
    }

    const char* modeStr = "off";
    if (mode == RepeatMode::TRACK) modeStr = "track";
    else if (mode == RepeatMode::CONTEXT) modeStr = "context";

    DEBUG_PRINTF("[SpotifyController] Set repeat: %s\n", modeStr);

    String endpoint = String(EP_REPEAT) + "?state=" + String(modeStr);
    String response;
    
    if (makeApiRequest(endpoint.c_str(), "PUT", "", response)) {
        m_currentTrack.setRepeatMode(mode);
        return true;
    }

    return false;
}

bool SpotifyController::makeApiRequest(const String& endpoint, const String& method, 
                                      const String& body, String& response) {
    String url = String(API_BASE) + endpoint;
    
    DEBUG_PRINTF("[SpotifyController] API %s: %s\n", method.c_str(), endpoint.c_str());

    m_http.begin(url);
    m_http.addHeader("Authorization", "Bearer " + m_accessToken);
    m_http.addHeader("Content-Type", "application/json");

    int httpCode;
    if (method == "GET") {
        httpCode = m_http.GET();
    } else if (method == "POST") {
        httpCode = m_http.POST(body);
    } else if (method == "PUT") {
        httpCode = m_http.PUT(body);
    } else {
        m_lastError = "Invalid HTTP method";
        m_http.end();
        return false;
    }

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_NO_CONTENT) {
        response = m_http.getString();
        m_http.end();
        return true;
    } else if (httpCode == HTTP_CODE_UNAUTHORIZED) {
        m_lastError = "Unauthorized - token may have expired";
        DEBUG_PRINTLN("[SpotifyController] ERROR: Token expired or invalid");
    } else {
        m_lastError = "HTTP error: " + String(httpCode);
        DEBUG_PRINTF("[SpotifyController] HTTP error: %d\n", httpCode);
    }

    m_http.end();
    return false;
}

bool SpotifyController::parseNowPlaying(const String& json) {
    DEBUG_PRINTLN("[SpotifyController] Parsing now playing response...");

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        m_lastError = "JSON parse error";
        DEBUG_PRINTF("[SpotifyController] JSON parse error: %s\n", error.c_str());
        return false;
    }

    // Extract track info
    JsonObject item = doc["item"];
    if (!item.isNull()) {
        m_currentTrack.setId(item["id"].as<String>());
        m_currentTrack.setName(item["name"].as<String>());
        
        // Artists (first one)
        JsonArray artists = item["artists"];
        if (artists.size() > 0) {
            m_currentTrack.setArtist(artists[0]["name"].as<String>());
        }
        
        // Album
        JsonObject album = item["album"];
        if (!album.isNull()) {
            m_currentTrack.setAlbum(album["name"].as<String>());
            
            // Album art (first image)
            JsonArray images = album["images"];
            if (images.size() > 0) {
                m_currentTrack.setAlbumArtUrl(images[0]["url"].as<String>());
            }
        }
        
        m_currentTrack.setDuration(item["duration_ms"].as<int>());
    }

    // Playback state
    bool isPlaying = doc["is_playing"].as<bool>();
    m_currentTrack.setPlaybackState(isPlaying ? PlaybackState::PLAYING : PlaybackState::PAUSED);

    // Progress
    m_currentTrack.setPosition(doc["progress_ms"].as<int>());

    // Context (playlist/album)
    JsonObject context = doc["context"];
    if (!context.isNull()) {
        m_currentTrack.setContextUri(context["uri"].as<String>());
    }

    DEBUG_PRINTF("[SpotifyController] Now playing: %s - %s\n", 
                 m_currentTrack.getArtist().c_str(), 
                 m_currentTrack.getName().c_str());

    return true;
}

void SpotifyController::loadAccessToken() {
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (!currentUser) {
        DEBUG_PRINTLN("[SpotifyController] No user logged in");
        return;
    }

    m_accessToken = DatabaseService::getInstance().getToken(
        currentUser->getId(), "spotify"
    );

    if (m_accessToken.length() > 0) {
        DEBUG_PRINTLN("[SpotifyController] Access token loaded from database");
    }
}

void SpotifyController::saveAccessToken() {
    User* currentUser = AuthService::getInstance().getCurrentUser();
    if (!currentUser) {
        DEBUG_PRINTLN("[SpotifyController] No user logged in, cannot save token");
        return;
    }

    DatabaseService::getInstance().saveToken(
        currentUser->getId(), "spotify", m_accessToken, "bearer"
    );

    DEBUG_PRINTLN("[SpotifyController] Access token saved to database");
}


