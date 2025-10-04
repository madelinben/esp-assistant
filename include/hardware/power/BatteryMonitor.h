/**
 * @file BatteryMonitor.h
 * @brief Battery monitoring and power management
 * 
 * Monitors battery level, charging status, and power states.
 * Part of Hardware Abstraction Layer (HAL).
 */

#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include "config/Config.h"

/**
 * @enum ChargingStatus
 * @brief Battery charging status
 */
enum class ChargingStatus {
    NOT_CHARGING,
    CHARGING,
    CHARGED,
    UNKNOWN
};

/**
 * @enum PowerMode
 * @brief Device power modes
 */
enum class PowerMode {
    ACTIVE,
    LOW_POWER,
    DEEP_SLEEP
};

/**
 * @class BatteryMonitor
 * @brief Singleton hardware driver for battery monitoring
 * 
 * Features:
 * - Battery level monitoring (ADC)
 * - Charging status detection
 * - Power mode management
 * - Low battery warnings
 * - Sleep modes
 */
class BatteryMonitor {
public:
    /**
     * @brief Get singleton instance
     */
    static BatteryMonitor& getInstance();

    /**
     * @brief Initialize battery monitoring
     * @return true if successful
     */
    bool init();

    /**
     * @brief Update battery readings (call periodically)
     */
    void update();

    /**
     * @brief Get battery level percentage
     * @return Battery level (0-100)
     */
    uint8_t getBatteryLevel();

    /**
     * @brief Get battery voltage in millivolts
     * @return Voltage in mV
     */
    uint16_t getBatteryVoltage();

    /**
     * @brief Check if battery is charging
     * @return true if charging
     */
    bool isCharging();

    /**
     * @brief Get charging status
     * @return ChargingStatus enum
     */
    ChargingStatus getChargingStatus();

    /**
     * @brief Check if battery is low
     * @param threshold Low battery threshold (default 20%)
     * @return true if battery is low
     */
    bool isBatteryLow(uint8_t threshold = 20);

    /**
     * @brief Check if battery is critical
     * @param threshold Critical battery threshold (default 5%)
     * @return true if battery is critical
     */
    bool isBatteryCritical(uint8_t threshold = 5);

    /**
     * @brief Set power mode
     * @param mode Power mode to set
     */
    void setPowerMode(PowerMode mode);

    /**
     * @brief Get current power mode
     * @return Current power mode
     */
    PowerMode getPowerMode() const { return m_powerMode; }

    /**
     * @brief Enter deep sleep mode
     * @param seconds Sleep duration in seconds (0 = indefinite)
     */
    void deepSleep(uint32_t seconds = 0);

    /**
     * @brief Enable/disable low power mode
     * @param enable true to enable
     */
    void setLowPowerMode(bool enable);

    /**
     * @brief Get time remaining estimate in minutes
     * @return Estimated minutes remaining (0 if unknown)
     */
    uint16_t getTimeRemaining();

private:
    BatteryMonitor();
    ~BatteryMonitor();
    BatteryMonitor(const BatteryMonitor&) = delete;
    BatteryMonitor& operator=(const BatteryMonitor&) = delete;

    uint16_t readBatteryADC();
    uint16_t voltageToPercentage(uint16_t voltage);

    uint8_t m_batteryLevel;
    uint16_t m_batteryVoltage;
    ChargingStatus m_chargingStatus;
    PowerMode m_powerMode;
    uint32_t m_lastUpdate;
    bool m_initialized;
    
    // Voltage reference points for 3.7V LiPo
    static constexpr uint16_t VOLTAGE_MAX = 4200;  // 4.2V (fully charged)
    static constexpr uint16_t VOLTAGE_MIN = 3000;  // 3.0V (empty)
    static constexpr uint16_t VOLTAGE_NOMINAL = 3700;  // 3.7V (nominal)
};

#endif // BATTERY_MONITOR_H


