/**
 * @file AudioDriver.h
 * @brief Audio driver for PCM5101 decoder and microphone
 * 
 * Manages audio output via PCM5101 I2S decoder and microphone input.
 * Part of Hardware Abstraction Layer (HAL).
 */

#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "config/Config.h"

/**
 * @enum AudioFormat
 * @brief Audio format types
 */
enum class AudioFormat {
    PCM_16BIT,
    PCM_24BIT,
    PCM_32BIT
};

/**
 * @class AudioDriver
 * @brief Singleton hardware driver for audio
 * 
 * Features:
 * - I2S audio output (PCM5101)
 * - Microphone input (I2S MEMS mic)
 * - Volume control
 * - Sample rate configuration
 * - Tone generation
 */
class AudioDriver {
public:
    /**
     * @brief Get singleton instance
     */
    static AudioDriver& getInstance();

    /**
     * @brief Initialize audio hardware
     * @param sampleRate Sample rate in Hz (default 44100)
     * @param bitsPerSample Bits per sample (16, 24, or 32)
     * @return true if successful
     */
    bool init(uint32_t sampleRate = 44100, uint8_t bitsPerSample = 16);

    /**
     * @brief Set output volume
     * @param volume Volume level (0-100)
     */
    void setVolume(uint8_t volume);

    /**
     * @brief Get current volume
     * @return Volume level (0-100)
     */
    uint8_t getVolume() const { return m_volume; }

    /**
     * @brief Play tone
     * @param frequency Frequency in Hz
     * @param duration Duration in milliseconds
     */
    void playTone(uint16_t frequency, uint32_t duration);

    /**
     * @brief Play notification sound
     */
    void playNotification();

    /**
     * @brief Write audio samples
     * @param samples Pointer to sample buffer
     * @param sampleCount Number of samples
     * @return Number of samples written
     */
    size_t writeSamples(const int16_t* samples, size_t sampleCount);

    /**
     * @brief Start microphone recording
     * @return true if successful
     */
    bool startRecording();

    /**
     * @brief Stop microphone recording
     */
    void stopRecording();

    /**
     * @brief Check if recording
     * @return true if recording
     */
    bool isRecording() const { return m_recording; }

    /**
     * @brief Read recorded samples
     * @param samples Output buffer
     * @param maxSamples Maximum samples to read
     * @return Number of samples read
     */
    size_t readSamples(int16_t* samples, size_t maxSamples);

    /**
     * @brief Mute/unmute audio output
     * @param mute true to mute
     */
    void setMute(bool mute);

    /**
     * @brief Check if muted
     * @return true if muted
     */
    bool isMuted() const { return m_muted; }

private:
    AudioDriver();
    ~AudioDriver();
    AudioDriver(const AudioDriver&) = delete;
    AudioDriver& operator=(const AudioDriver&) = delete;

    bool initI2SOutput();
    bool initI2SInput();
    void generateToneSamples(int16_t* buffer, size_t sampleCount, uint16_t frequency);

    i2s_port_t m_i2sPortOut;
    i2s_port_t m_i2sPortIn;
    uint32_t m_sampleRate;
    uint8_t m_bitsPerSample;
    uint8_t m_volume;
    bool m_muted;
    bool m_recording;
    bool m_initialized;
};

#endif // AUDIO_DRIVER_H


