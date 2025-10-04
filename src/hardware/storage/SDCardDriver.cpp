/**
 * @file SDCardDriver.cpp
 * @brief Implementation of SDCardDriver
 */

#include "hardware/storage/SDCardDriver.h"

SDCardDriver& SDCardDriver::getInstance() {
    static SDCardDriver instance;
    return instance;
}

SDCardDriver::SDCardDriver()
    : m_mounted(false)
    , m_spi(HSPI) {  // Use HSPI for SD card
}

SDCardDriver::~SDCardDriver() {
    unmount();
}

bool SDCardDriver::init() {
    if (m_mounted) {
        DEBUG_PRINTLN("[SDCardDriver] Already mounted");
        return true;
    }

    DEBUG_PRINTLN("[SDCardDriver] Initializing SD card...");

    // Initialize SPI for SD card
    m_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

    // Try to mount SD card
    if (!SD.begin(SD_CS, m_spi, 25000000)) {  // 25MHz SPI frequency
        DEBUG_PRINTLN("[SDCardDriver] ERROR: Failed to mount SD card");
        DEBUG_PRINTLN("[SDCardDriver] Check:");
        DEBUG_PRINTLN("[SDCardDriver]   1. SD card is inserted");
        DEBUG_PRINTLN("[SDCardDriver]   2. SD card is formatted as FAT32");
        DEBUG_PRINTLN("[SDCardDriver]   3. Connections are correct");
        return false;
    }

    // Get card type
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        DEBUG_PRINTLN("[SDCardDriver] ERROR: No SD card detected");
        return false;
    }

    // Print card info
    DEBUG_PRINT("[SDCardDriver] Card type: ");
    switch (cardType) {
        case CARD_MMC:
            DEBUG_PRINTLN("MMC");
            break;
        case CARD_SD:
            DEBUG_PRINTLN("SD");
            break;
        case CARD_SDHC:
            DEBUG_PRINTLN("SDHC");
            break;
        default:
            DEBUG_PRINTLN("Unknown");
    }

    // Print card size
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    DEBUG_PRINTF("[SDCardDriver] Card size: %llu MB\n", cardSize);

    // Print volume info
    uint64_t totalBytes = SD.totalBytes();
    uint64_t usedBytes = SD.usedBytes();
    DEBUG_PRINTF("[SDCardDriver] Total space: %llu MB\n", totalBytes / (1024 * 1024));
    DEBUG_PRINTF("[SDCardDriver] Used space: %llu MB\n", usedBytes / (1024 * 1024));
    DEBUG_PRINTF("[SDCardDriver] Free space: %llu MB\n", (totalBytes - usedBytes) / (1024 * 1024));

    // Create required directories
    if (!dirExists("/database")) {
        createDir("/database");
        DEBUG_PRINTLN("[SDCardDriver] Created /database directory");
    }
    if (!dirExists("/assets")) {
        createDir("/assets");
        DEBUG_PRINTLN("[SDCardDriver] Created /assets directory");
    }
    if (!dirExists("/config")) {
        createDir("/config");
        DEBUG_PRINTLN("[SDCardDriver] Created /config directory");
    }

    m_mounted = true;
    DEBUG_PRINTLN("[SDCardDriver] SD card mounted successfully");
    return true;
}

void SDCardDriver::unmount() {
    if (m_mounted) {
        SD.end();
        m_mounted = false;
        DEBUG_PRINTLN("[SDCardDriver] SD card unmounted");
    }
}

uint64_t SDCardDriver::getTotalSpace() {
    if (!m_mounted) return 0;
    return SD.totalBytes();
}

uint64_t SDCardDriver::getUsedSpace() {
    if (!m_mounted) return 0;
    return SD.usedBytes();
}

uint64_t SDCardDriver::getFreeSpace() {
    if (!m_mounted) return 0;
    return SD.totalBytes() - SD.usedBytes();
}

bool SDCardDriver::fileExists(const char* path) {
    if (!m_mounted) return false;
    return SD.exists(path);
}

bool SDCardDriver::dirExists(const char* path) {
    if (!m_mounted) return false;
    File dir = SD.open(path);
    if (!dir) return false;
    bool isDir = dir.isDirectory();
    dir.close();
    return isDir;
}

bool SDCardDriver::createDir(const char* path) {
    if (!m_mounted) return false;
    return SD.mkdir(path);
}

bool SDCardDriver::deleteFile(const char* path) {
    if (!m_mounted) return false;
    return SD.remove(path);
}

bool SDCardDriver::deleteDir(const char* path) {
    if (!m_mounted) return false;
    return SD.rmdir(path);
}

File SDCardDriver::openFile(const char* path, const char* mode) {
    if (!m_mounted) return File();
    return SD.open(path, mode);
}

String SDCardDriver::readFile(const char* path) {
    if (!m_mounted) return "";

    File file = SD.open(path, FILE_READ);
    if (!file) {
        DEBUG_PRINTF("[SDCardDriver] Failed to open file: %s\n", path);
        return "";
    }

    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();

    return content;
}

bool SDCardDriver::writeFile(const char* path, const String& content, bool append) {
    if (!m_mounted) return false;

    const char* mode = append ? FILE_APPEND : FILE_WRITE;
    File file = SD.open(path, mode);
    if (!file) {
        DEBUG_PRINTF("[SDCardDriver] Failed to open file for writing: %s\n", path);
        return false;
    }

    size_t bytesWritten = file.print(content);
    file.close();

    return bytesWritten == content.length();
}

int SDCardDriver::listFiles(const char* path, String* files, int maxFiles) {
    if (!m_mounted || !files || maxFiles <= 0) return 0;

    File dir = SD.open(path);
    if (!dir || !dir.isDirectory()) {
        return 0;
    }

    int count = 0;
    File file = dir.openNextFile();
    while (file && count < maxFiles) {
        if (!file.isDirectory()) {
            files[count++] = String(file.name());
        }
        file = dir.openNextFile();
    }

    dir.close();
    return count;
}


