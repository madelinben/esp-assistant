/**
 * @file SDCardDriver.h
 * @brief SD card driver for TF card slot - Hardware Abstraction Layer
 * 
 * Manages SD card initialization, mounting, and file operations.
 * Supports FAT32 format for database and asset storage.
 * Part of Hardware Abstraction Layer (HAL).
 */

#ifndef SDCARD_DRIVER_H
#define SDCARD_DRIVER_H

#include <Arduino.h>
#include <SD.h>
#include <FS.h>
#include "config/Config.h"

/**
 * @class SDCardDriver
 * @brief Singleton hardware driver for SD card
 * 
 * Handles SD card operations:
 * - Mount/unmount
 * - File operations (open, read, write, delete)
 * - Directory operations
 * - Space management
 */
class SDCardDriver {
public:
    /**
     * @brief Get singleton instance
     */
    static SDCardDriver& getInstance();

    /**
     * @brief Initialize and mount SD card
     * @return true if successful
     */
    bool init();

    /**
     * @brief Check if SD card is mounted
     * @return true if mounted
     */
    bool isMounted() const { return m_mounted; }

    /**
     * @brief Unmount SD card
     */
    void unmount();

    /**
     * @brief Get total space in bytes
     * @return Total space or 0 on error
     */
    uint64_t getTotalSpace();

    /**
     * @brief Get used space in bytes
     * @return Used space or 0 on error
     */
    uint64_t getUsedSpace();

    /**
     * @brief Get free space in bytes
     * @return Free space or 0 on error
     */
    uint64_t getFreeSpace();

    /**
     * @brief Check if file exists
     * @param path File path
     * @return true if exists
     */
    bool fileExists(const char* path);

    /**
     * @brief Check if directory exists
     * @param path Directory path
     * @return true if exists
     */
    bool dirExists(const char* path);

    /**
     * @brief Create directory
     * @param path Directory path
     * @return true if successful
     */
    bool createDir(const char* path);

    /**
     * @brief Delete file
     * @param path File path
     * @return true if successful
     */
    bool deleteFile(const char* path);

    /**
     * @brief Delete directory (recursive)
     * @param path Directory path
     * @return true if successful
     */
    bool deleteDir(const char* path);

    /**
     * @brief Open file for reading
     * @param path File path
     * @return File object
     */
    File openFile(const char* path, const char* mode = FILE_READ);

    /**
     * @brief Read file contents to string
     * @param path File path
     * @return File contents or empty string
     */
    String readFile(const char* path);

    /**
     * @brief Write string to file
     * @param path File path
     * @param content Content to write
     * @param append If true, append to file
     * @return true if successful
     */
    bool writeFile(const char* path, const String& content, bool append = false);

    /**
     * @brief List files in directory
     * @param path Directory path
     * @param files Output array of file names
     * @param maxFiles Maximum files to list
     * @return Number of files found
     */
    int listFiles(const char* path, String* files, int maxFiles);

private:
    SDCardDriver();
    ~SDCardDriver();
    SDCardDriver(const SDCardDriver&) = delete;
    SDCardDriver& operator=(const SDCardDriver&) = delete;

    bool m_mounted;
    SPIClass m_spi;
};

#endif // SDCARD_DRIVER_H


