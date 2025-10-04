/**
 * @file BatteryMonitor.cpp
 * @brief Implementation of BatteryMonitor
 */

#include "hardware/power/BatteryMonitor.h"
#include <esp_sleep.h>
#include <esp_pm.h>

BatteryMonitor& BatteryMonitor::getInstance() {
    static BatteryMonitor instance;
    return instance;
}

BatteryMonitor::BatteryMonitor()
    : m_batteryLevel(0)
    , m_batteryVoltage(0)
    , m_chargingStatus(ChargingStatus::UNKNOWN)
    , m_powerMode(PowerMode::ACTIVE)
    , m_lastUpdate(0)
    , m_initialized(false) {
}

BatteryMonitor::~BatteryMonitor() {
}

bool BatteryMonitor::init() {
    if (m_initialized) {
        return true;
    }

    DEBUG_PRINTLN("[BatteryMonitor] Initializing...");

    // Configure ADC for battery voltage reading
    analogReadResolution(12);  // 12-bit ADC (0-4095)
    analogSetAttenuation(ADC_11db);  // Full range ~3.3V
    
    // Configure charging status pin
    pinMode(CHARGE_STATUS, INPUT);

    // Initial update
    update();

    m_initialized = true;
    DEBUG_PRINTF("[BatteryMonitor] Initialized. Battery: %d%%, %dmV, Charging: %s\n", 
                 m_batteryLevel, m_batteryVoltage, 
                 isCharging() ? "YES" : "NO");
    return true;
}

void BatteryMonitor::update() {
    uint32_t currentTime = millis();
    
    // Don't update too frequently (every 1 second)
    if (currentTime - m_lastUpdate < 1000) {
        return;
    }
    
    m_lastUpdate = currentTime;

    // Read battery voltage
    m_batteryVoltage = readBatteryADC();
    m_batteryLevel = voltageToPercentage(m_batteryVoltage);

    // Read charging status
    bool chargingPin = digitalRead(CHARGE_STATUS) == LOW;  // Active low
    
    if (chargingPin) {
        if (m_batteryLevel >= 95) {
            m_chargingStatus = ChargingStatus::CHARGED;
        } else {
            m_chargingStatus = ChargingStatus::CHARGING;
        }
    } else {
        m_chargingStatus = ChargingStatus::NOT_CHARGING;
    }
}

uint8_t BatteryMonitor::getBatteryLevel() {
    return m_batteryLevel;
}

uint16_t BatteryMonitor::getBatteryVoltage() {
    return m_batteryVoltage;
}

bool BatteryMonitor::isCharging() {
    return m_chargingStatus == ChargingStatus::CHARGING ||
           m_chargingStatus == ChargingStatus::CHARGED;
}

ChargingStatus BatteryMonitor::getChargingStatus() {
    return m_chargingStatus;
}

bool BatteryMonitor::isBatteryLow(uint8_t threshold) {
    return m_batteryLevel < threshold && !isCharging();
}

bool BatteryMonitor::isBatteryCritical(uint8_t threshold) {
    return m_batteryLevel < threshold && !isCharging();
}

void BatteryMonitor::setPowerMode(PowerMode mode) {
    if (m_powerMode == mode) return;

    DEBUG_PRINTF("[BatteryMonitor] Setting power mode: %d\n", (int)mode);
    
    m_powerMode = mode;

    switch (mode) {
        case PowerMode::ACTIVE:
            // Full performance
            setCpuFrequencyMhz(240);
            break;
            
        case PowerMode::LOW_POWER:
            // Reduced performance to save power
            setCpuFrequencyMhz(80);
            break;
            
        case PowerMode::DEEP_SLEEP:
            // Will enter deep sleep
            break;
    }
}

void BatteryMonitor::deepSleep(uint32_t seconds) {
    DEBUG_PRINTF("[BatteryMonitor] Entering deep sleep for %d seconds\n", seconds);

    // Configure wake-up source
    if (seconds > 0) {
        esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);  // Convert to microseconds
    }
    
    // TODO: Configure touch wake-up
    // esp_sleep_enable_ext0_wakeup(TOUCH_INT, 0);  // Wake on touch interrupt

    // Enter deep sleep
    esp_deep_sleep_start();
}

void BatteryMonitor::setLowPowerMode(bool enable) {
    if (enable) {
        setPowerMode(PowerMode::LOW_POWER);
    } else {
        setPowerMode(PowerMode::ACTIVE);
    }
}

uint16_t BatteryMonitor::getTimeRemaining() {
    if (isCharging()) {
        return 0;  // Charging, no time remaining
    }

    // Rough estimate based on battery level
    // Assuming ~8 hours at 100%
    const uint16_t MAX_MINUTES = 8 * 60;
    return (m_batteryLevel * MAX_MINUTES) / 100;
}

uint16_t BatteryMonitor::readBatteryADC() {
    // Read ADC multiple times and average
    const int numReadings = 10;
    uint32_t sum = 0;
    
    for (int i = 0; i < numReadings; i++) {
        sum += analogRead(BATTERY_ADC);
        delay(10);
    }
    
    uint16_t adcValue = sum / numReadings;

    // Convert ADC value to voltage
    // ADC range: 0-4095 (12-bit)
    // Voltage range: 0-3.3V (with 11db attenuation)
    // Battery voltage is divided by resistor divider (typically 2:1)
    
    // Assuming 2:1 voltage divider:
    // Battery voltage = (ADC * 3.3 / 4095) * 2
    float voltage = (adcValue * 3.3f / 4095.0f) * 2.0f;
    
    // Convert to millivolts
    return (uint16_t)(voltage * 1000.0f);
}

uint16_t BatteryMonitor::voltageToPercentage(uint16_t voltage) {
    // Clamp to valid range
    if (voltage >= VOLTAGE_MAX) return 100;
    if (voltage <= VOLTAGE_MIN) return 0;

    // Linear interpolation between min and max
    // Note: LiPo discharge curve is not linear, but this is a simple approximation
    uint16_t range = VOLTAGE_MAX - VOLTAGE_MIN;
    uint16_t adjusted = voltage - VOLTAGE_MIN;
    
    return (adjusted * 100) / range;
}


