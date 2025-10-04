/**
 * @file CryptoUtils.h
 * @brief Cryptographic utilities for encryption/decryption
 * 
 * Provides AES-256 encryption for secure storage of sensitive data.
 * Part of MVC architecture - Utility layer.
 */

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <Arduino.h>
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "mbedtls/pkcs5.h"

/**
 * @class CryptoUtils
 * @brief Cryptographic utilities
 * 
 * Features:
 * - AES-256 encryption/decryption
 * - PBKDF2 key derivation
 * - Base64 encoding/decoding
 * - Secure key generation
 * - IV (Initialization Vector) management
 */
class CryptoUtils {
public:
    /**
     * @brief Encrypt data using AES-256-CBC
     * @param plaintext Data to encrypt
     * @param key Encryption key (32 bytes for AES-256)
     * @param ciphertext Output encrypted data (Base64 encoded)
     * @return true if successful
     */
    static bool encrypt(const String& plaintext, const uint8_t* key, String& ciphertext);

    /**
     * @brief Decrypt data using AES-256-CBC
     * @param ciphertext Encrypted data (Base64 encoded)
     * @param key Decryption key (32 bytes for AES-256)
     * @param plaintext Output decrypted data
     * @return true if successful
     */
    static bool decrypt(const String& ciphertext, const uint8_t* key, String& plaintext);

    /**
     * @brief Derive encryption key from password using PBKDF2
     * @param password User password
     * @param salt Salt value (should be unique per user)
     * @param iterations Number of iterations (higher = more secure but slower)
     * @param keyLength Output key length in bytes
     * @param derivedKey Output derived key buffer
     * @return true if successful
     */
    static bool deriveKey(const String& password, const String& salt, 
                         int iterations, int keyLength, uint8_t* derivedKey);

    /**
     * @brief Generate random IV (Initialization Vector)
     * @param iv Output IV buffer (16 bytes for AES)
     */
    static void generateIV(uint8_t* iv);

    /**
     * @brief Generate random salt
     * @param salt Output salt buffer
     * @param length Salt length in bytes
     */
    static void generateSalt(uint8_t* salt, int length);

    /**
     * @brief Encode binary data to Base64
     * @param data Binary data
     * @param length Data length
     * @return Base64 encoded string
     */
    static String base64Encode(const uint8_t* data, size_t length);

    /**
     * @brief Decode Base64 string to binary
     * @param base64 Base64 encoded string
     * @param output Output buffer
     * @param maxLength Maximum output length
     * @return Actual output length, or -1 on error
     */
    static int base64Decode(const String& base64, uint8_t* output, size_t maxLength);

    /**
     * @brief Calculate SHA-256 hash
     * @param data Input data
     * @param hash Output hash (32 bytes)
     * @return true if successful
     */
    static bool sha256(const String& data, uint8_t* hash);

    /**
     * @brief Convert hex string to bytes
     * @param hex Hex string
     * @param bytes Output bytes
     * @param maxLength Maximum output length
     * @return Number of bytes converted
     */
    static int hexToBytes(const String& hex, uint8_t* bytes, int maxLength);

    /**
     * @brief Convert bytes to hex string
     * @param bytes Input bytes
     * @param length Byte length
     * @return Hex string
     */
    static String bytesToHex(const uint8_t* bytes, int length);

private:
    // AES block size (16 bytes)
    static const int AES_BLOCK_SIZE = 16;
    
    // AES-256 key size (32 bytes)
    static const int AES_KEY_SIZE = 32;
    
    // Default PBKDF2 iterations
    static const int DEFAULT_PBKDF2_ITERATIONS = 10000;

    // Helper: Apply PKCS7 padding
    static void applyPadding(uint8_t* data, int dataLength, int blockSize, int& paddedLength);
    
    // Helper: Remove PKCS7 padding
    static int removePadding(const uint8_t* data, int dataLength);
};

#endif // CRYPTO_UTILS_H


