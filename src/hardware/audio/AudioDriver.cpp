/**
 * @file AudioDriver.cpp
 * @brief Implementation of AudioDriver
 */

#include "hardware/audio/AudioDriver.h"
#include <cmath>

#define I2S_PORT_OUT I2S_NUM_0
#define I2S_PORT_IN  I2S_NUM_1

AudioDriver& AudioDriver::getInstance() {
    static AudioDriver instance;
    return instance;
}

AudioDriver::AudioDriver()
    : m_i2sPortOut(I2S_PORT_OUT)
    , m_i2sPortIn(I2S_PORT_IN)
    , m_sampleRate(44100)
    , m_bitsPerSample(16)
    , m_volume(70)
    , m_muted(false)
    , m_recording(false)
    , m_initialized(false) {
}

AudioDriver::~AudioDriver() {
    if (m_initialized) {
        i2s_driver_uninstall(m_i2sPortOut);
        i2s_driver_uninstall(m_i2sPortIn);
    }
}

bool AudioDriver::init(uint32_t sampleRate, uint8_t bitsPerSample) {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[AudioDriver] Initializing audio...");

    m_sampleRate = sampleRate;
    m_bitsPerSample = bitsPerSample;

    // Initialize I2S output (PCM5101)
    if (!initI2SOutput()) {
        DEBUG_PRINTLN("[AudioDriver] ERROR: Failed to initialize I2S output");
        return false;
    }

    // Initialize I2S input (microphone)
    if (!initI2SInput()) {
        DEBUG_PRINTLN("[AudioDriver] WARNING: Failed to initialize I2S input (microphone)");
        // Continue anyway - output is more important
    }

    m_initialized = true;
    DEBUG_PRINTF("[AudioDriver] Initialized at %d Hz, %d bits\n", m_sampleRate, m_bitsPerSample);
    return true;
}

bool AudioDriver::initI2SOutput() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = m_sampleRate,
        .bits_per_sample = (i2s_bits_per_sample_t)m_bitsPerSample,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    esp_err_t err = i2s_driver_install(m_i2sPortOut, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        DEBUG_PRINTF("[AudioDriver] I2S driver install failed: %d\n", err);
        return false;
    }

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    err = i2s_set_pin(m_i2sPortOut, &pin_config);
    if (err != ESP_OK) {
        DEBUG_PRINTF("[AudioDriver] I2S set pin failed: %d\n", err);
        i2s_driver_uninstall(m_i2sPortOut);
        return false;
    }

    // Set initial volume
    i2s_set_clk(m_i2sPortOut, m_sampleRate, (i2s_bits_per_sample_t)m_bitsPerSample, I2S_CHANNEL_STEREO);

    DEBUG_PRINTLN("[AudioDriver] I2S output initialized");
    return true;
}

bool AudioDriver::initI2SInput() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = m_sampleRate,
        .bits_per_sample = (i2s_bits_per_sample_t)m_bitsPerSample,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    esp_err_t err = i2s_driver_install(m_i2sPortIn, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        DEBUG_PRINTF("[AudioDriver] I2S input driver install failed: %d\n", err);
        return false;
    }

    i2s_pin_config_t pin_config = {
        .bck_io_num = MIC_CLK,
        .ws_io_num = I2S_PIN_NO_CHANGE,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = MIC_DATA
    };

    err = i2s_set_pin(m_i2sPortIn, &pin_config);
    if (err != ESP_OK) {
        DEBUG_PRINTF("[AudioDriver] I2S input set pin failed: %d\n", err);
        i2s_driver_uninstall(m_i2sPortIn);
        return false;
    }

    DEBUG_PRINTLN("[AudioDriver] I2S input initialized");
    return true;
}

void AudioDriver::setVolume(uint8_t volume) {
    if (volume > 100) volume = 100;
    m_volume = volume;
    DEBUG_PRINTF("[AudioDriver] Volume set to %d%%\n", volume);
    
    // Note: PCM5101 doesn't have built-in volume control
    // Volume is controlled by scaling samples before writing
}

void AudioDriver::playTone(uint16_t frequency, uint32_t duration) {
    if (!m_initialized || m_muted) return;

    const size_t bufferSize = 512;
    int16_t buffer[bufferSize];

    uint32_t samplesNeeded = (m_sampleRate * duration) / 1000;
    uint32_t samplesWritten = 0;

    while (samplesWritten < samplesNeeded) {
        size_t samplesToWrite = min((size_t)(samplesNeeded - samplesWritten), bufferSize);
        generateToneSamples(buffer, samplesToWrite, frequency);
        
        size_t bytesWritten;
        i2s_write(m_i2sPortOut, buffer, samplesToWrite * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
        
        samplesWritten += samplesToWrite;
    }
}

void AudioDriver::generateToneSamples(int16_t* buffer, size_t sampleCount, uint16_t frequency) {
    static uint32_t phase = 0;
    float phaseIncrement = (2.0f * PI * frequency) / m_sampleRate;
    
    for (size_t i = 0; i < sampleCount; i++) {
        float sample = sin(phase * phaseIncrement);
        // Apply volume
        sample *= (m_volume / 100.0f);
        // Convert to 16-bit PCM
        buffer[i] = (int16_t)(sample * 32767.0f);
        phase++;
    }
}

void AudioDriver::playNotification() {
    // Play a pleasant notification tone (two beeps)
    playTone(800, 100);
    delay(50);
    playTone(1000, 100);
}

size_t AudioDriver::writeSamples(const int16_t* samples, size_t sampleCount) {
    if (!m_initialized || m_muted || !samples) return 0;

    // Apply volume scaling
    int16_t* scaledSamples = new int16_t[sampleCount];
    float volumeScale = m_volume / 100.0f;
    
    for (size_t i = 0; i < sampleCount; i++) {
        scaledSamples[i] = (int16_t)(samples[i] * volumeScale);
    }

    size_t bytesWritten;
    i2s_write(m_i2sPortOut, scaledSamples, sampleCount * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    
    delete[] scaledSamples;
    return bytesWritten / sizeof(int16_t);
}

bool AudioDriver::startRecording() {
    if (!m_initialized || m_recording) return false;

    DEBUG_PRINTLN("[AudioDriver] Starting recording...");
    i2s_zero_dma_buffer(m_i2sPortIn);
    m_recording = true;
    return true;
}

void AudioDriver::stopRecording() {
    if (!m_recording) return;

    DEBUG_PRINTLN("[AudioDriver] Stopping recording...");
    m_recording = false;
}

size_t AudioDriver::readSamples(int16_t* samples, size_t maxSamples) {
    if (!m_initialized || !m_recording || !samples) return 0;

    size_t bytesRead;
    i2s_read(m_i2sPortIn, samples, maxSamples * sizeof(int16_t), &bytesRead, portMAX_DELAY);
    
    return bytesRead / sizeof(int16_t);
}

void AudioDriver::setMute(bool mute) {
    m_muted = mute;
    DEBUG_PRINTF("[AudioDriver] Mute: %s\n", mute ? "ON" : "OFF");
}


