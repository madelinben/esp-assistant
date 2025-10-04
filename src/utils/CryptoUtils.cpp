/**
 * @file CryptoUtils.cpp
 * @brief Implementation of CryptoUtils
 */

#include "utils/CryptoUtils.h"
#include "esp_random.h"

// Base64 encoding table
static const char BASE64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

bool CryptoUtils::encrypt(const String& plaintext, const uint8_t* key, String& ciphertext) {
    if (plaintext.length() == 0 || !key) {
        return false;
    }

    // Generate random IV
    uint8_t iv[AES_BLOCK_SIZE];
    generateIV(iv);

    // Prepare plaintext with padding
    int plaintextLen = plaintext.length();
    int paddedLen;
    uint8_t* paddedData = new uint8_t[plaintextLen + AES_BLOCK_SIZE];
    memcpy(paddedData, plaintext.c_str(), plaintextLen);
    applyPadding(paddedData, plaintextLen, AES_BLOCK_SIZE, paddedLen);

    // Allocate buffer for ciphertext
    uint8_t* encryptedData = new uint8_t[paddedLen];

    // Initialize AES context
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    // Set encryption key
    int ret = mbedtls_aes_setkey_enc(&aes, key, AES_KEY_SIZE * 8);
    if (ret != 0) {
        DEBUG_PRINTF("[CryptoUtils] AES setkey failed: %d\n", ret);
        mbedtls_aes_free(&aes);
        delete[] paddedData;
        delete[] encryptedData;
        return false;
    }

    // Encrypt using CBC mode
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLen, iv, paddedData, encryptedData);

    mbedtls_aes_free(&aes);
    delete[] paddedData;

    if (ret != 0) {
        DEBUG_PRINTF("[CryptoUtils] AES encryption failed: %d\n", ret);
        delete[] encryptedData;
        return false;
    }

    // Combine IV + encrypted data
    int totalLen = AES_BLOCK_SIZE + paddedLen;
    uint8_t* combined = new uint8_t[totalLen];
    memcpy(combined, iv, AES_BLOCK_SIZE);
    memcpy(combined + AES_BLOCK_SIZE, encryptedData, paddedLen);

    // Encode to Base64
    ciphertext = base64Encode(combined, totalLen);

    delete[] encryptedData;
    delete[] combined;

    return true;
}

bool CryptoUtils::decrypt(const String& ciphertext, const uint8_t* key, String& plaintext) {
    if (ciphertext.length() == 0 || !key) {
        return false;
    }

    // Decode from Base64
    uint8_t* combined = new uint8_t[ciphertext.length()];
    int combinedLen = base64Decode(ciphertext, combined, ciphertext.length());

    if (combinedLen < AES_BLOCK_SIZE) {
        DEBUG_PRINTLN("[CryptoUtils] Invalid ciphertext length");
        delete[] combined;
        return false;
    }

    // Extract IV and encrypted data
    uint8_t iv[AES_BLOCK_SIZE];
    memcpy(iv, combined, AES_BLOCK_SIZE);

    int encryptedLen = combinedLen - AES_BLOCK_SIZE;
    uint8_t* encryptedData = new uint8_t[encryptedLen];
    memcpy(encryptedData, combined + AES_BLOCK_SIZE, encryptedLen);
    delete[] combined;

    // Allocate buffer for decrypted data
    uint8_t* decryptedData = new uint8_t[encryptedLen];

    // Initialize AES context
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);

    // Set decryption key
    int ret = mbedtls_aes_setkey_dec(&aes, key, AES_KEY_SIZE * 8);
    if (ret != 0) {
        DEBUG_PRINTF("[CryptoUtils] AES setkey failed: %d\n", ret);
        mbedtls_aes_free(&aes);
        delete[] encryptedData;
        delete[] decryptedData;
        return false;
    }

    // Decrypt using CBC mode
    ret = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, encryptedLen, iv, encryptedData, decryptedData);

    mbedtls_aes_free(&aes);
    delete[] encryptedData;

    if (ret != 0) {
        DEBUG_PRINTF("[CryptoUtils] AES decryption failed: %d\n", ret);
        delete[] decryptedData;
        return false;
    }

    // Remove padding
    int plaintextLen = removePadding(decryptedData, encryptedLen);
    if (plaintextLen < 0) {
        DEBUG_PRINTLN("[CryptoUtils] Invalid padding");
        delete[] decryptedData;
        return false;
    }

    // Convert to string
    plaintext = "";
    for (int i = 0; i < plaintextLen; i++) {
        plaintext += (char)decryptedData[i];
    }

    delete[] decryptedData;
    return true;
}

bool CryptoUtils::deriveKey(const String& password, const String& salt, 
                           int iterations, int keyLength, uint8_t* derivedKey) {
    if (password.length() == 0 || salt.length() == 0 || !derivedKey) {
        return false;
    }

    mbedtls_md_context_t md_ctx;
    mbedtls_md_init(&md_ctx);

    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md_info) {
        DEBUG_PRINTLN("[CryptoUtils] Failed to get MD info");
        return false;
    }

    int ret = mbedtls_md_setup(&md_ctx, md_info, 1);
    if (ret != 0) {
        DEBUG_PRINTF("[CryptoUtils] MD setup failed: %d\n", ret);
        mbedtls_md_free(&md_ctx);
        return false;
    }

    ret = mbedtls_pkcs5_pbkdf2_hmac(&md_ctx,
                                     (const unsigned char*)password.c_str(), password.length(),
                                     (const unsigned char*)salt.c_str(), salt.length(),
                                     iterations,
                                     keyLength,
                                     derivedKey);

    mbedtls_md_free(&md_ctx);

    if (ret != 0) {
        DEBUG_PRINTF("[CryptoUtils] PBKDF2 failed: %d\n", ret);
        return false;
    }

    return true;
}

void CryptoUtils::generateIV(uint8_t* iv) {
    // Use ESP32's hardware random number generator
    esp_fill_random(iv, AES_BLOCK_SIZE);
}

void CryptoUtils::generateSalt(uint8_t* salt, int length) {
    // Use ESP32's hardware random number generator
    esp_fill_random(salt, length);
}

String CryptoUtils::base64Encode(const uint8_t* data, size_t length) {
    String encoded = "";
    int i = 0;
    uint8_t byte3[3];
    uint8_t byte4[4];

    while (length--) {
        byte3[i++] = *(data++);
        if (i == 3) {
            byte4[0] = (byte3[0] & 0xfc) >> 2;
            byte4[1] = ((byte3[0] & 0x03) << 4) + ((byte3[1] & 0xf0) >> 4);
            byte4[2] = ((byte3[1] & 0x0f) << 2) + ((byte3[2] & 0xc0) >> 6);
            byte4[3] = byte3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                encoded += BASE64_CHARS[byte4[i]];
            }
            i = 0;
        }
    }

    if (i > 0) {
        for (int j = i; j < 3; j++) {
            byte3[j] = '\0';
        }

        byte4[0] = (byte3[0] & 0xfc) >> 2;
        byte4[1] = ((byte3[0] & 0x03) << 4) + ((byte3[1] & 0xf0) >> 4);
        byte4[2] = ((byte3[1] & 0x0f) << 2) + ((byte3[2] & 0xc0) >> 6);

        for (int j = 0; j < i + 1; j++) {
            encoded += BASE64_CHARS[byte4[j]];
        }

        while (i++ < 3) {
            encoded += '=';
        }
    }

    return encoded;
}

int CryptoUtils::base64Decode(const String& base64, uint8_t* output, size_t maxLength) {
    int length = base64.length();
    int i = 0;
    int j = 0;
    int outputIdx = 0;
    uint8_t byte4[4];
    uint8_t byte3[3];

    while (length-- && base64[i] != '=') {
        if (outputIdx >= (int)maxLength) {
            return -1;  // Buffer too small
        }

        // Find character in base64 table
        int val = -1;
        char ch = base64[i];
        for (int k = 0; k < 64; k++) {
            if (BASE64_CHARS[k] == ch) {
                val = k;
                break;
            }
        }

        if (val < 0) {
            i++;
            continue;
        }

        byte4[j++] = val;

        if (j == 4) {
            byte3[0] = (byte4[0] << 2) + ((byte4[1] & 0x30) >> 4);
            byte3[1] = ((byte4[1] & 0xf) << 4) + ((byte4[2] & 0x3c) >> 2);
            byte3[2] = ((byte4[2] & 0x3) << 6) + byte4[3];

            for (j = 0; j < 3; j++) {
                output[outputIdx++] = byte3[j];
            }
            j = 0;
        }
        i++;
    }

    if (j > 0) {
        for (int k = j; k < 4; k++) {
            byte4[k] = 0;
        }

        byte3[0] = (byte4[0] << 2) + ((byte4[1] & 0x30) >> 4);
        byte3[1] = ((byte4[1] & 0xf) << 4) + ((byte4[2] & 0x3c) >> 2);

        for (int k = 0; k < j - 1; k++) {
            output[outputIdx++] = byte3[k];
        }
    }

    return outputIdx;
}

bool CryptoUtils::sha256(const String& data, uint8_t* hash) {
    if (!hash) {
        return false;
    }

    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);

    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md_info) {
        return false;
    }

    int ret = mbedtls_md_setup(&ctx, md_info, 0);
    if (ret != 0) {
        mbedtls_md_free(&ctx);
        return false;
    }

    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    mbedtls_md_finish(&ctx, hash);

    mbedtls_md_free(&ctx);
    return true;
}

int CryptoUtils::hexToBytes(const String& hex, uint8_t* bytes, int maxLength) {
    int length = hex.length();
    if (length % 2 != 0 || length / 2 > maxLength) {
        return -1;
    }

    int byteCount = 0;
    for (int i = 0; i < length; i += 2) {
        char c1 = hex[i];
        char c2 = hex[i + 1];

        uint8_t b1 = (c1 >= '0' && c1 <= '9') ? (c1 - '0') :
                     (c1 >= 'a' && c1 <= 'f') ? (c1 - 'a' + 10) :
                     (c1 >= 'A' && c1 <= 'F') ? (c1 - 'A' + 10) : 0;

        uint8_t b2 = (c2 >= '0' && c2 <= '9') ? (c2 - '0') :
                     (c2 >= 'a' && c2 <= 'f') ? (c2 - 'a' + 10) :
                     (c2 >= 'A' && c2 <= 'F') ? (c2 - 'A' + 10) : 0;

        bytes[byteCount++] = (b1 << 4) | b2;
    }

    return byteCount;
}

String CryptoUtils::bytesToHex(const uint8_t* bytes, int length) {
    String hex = "";
    for (int i = 0; i < length; i++) {
        char buf[3];
        sprintf(buf, "%02x", bytes[i]);
        hex += buf;
    }
    return hex;
}

void CryptoUtils::applyPadding(uint8_t* data, int dataLength, int blockSize, int& paddedLength) {
    // PKCS7 padding
    int padding = blockSize - (dataLength % blockSize);
    paddedLength = dataLength + padding;

    for (int i = dataLength; i < paddedLength; i++) {
        data[i] = padding;
    }
}

int CryptoUtils::removePadding(const uint8_t* data, int dataLength) {
    if (dataLength == 0) {
        return -1;
    }

    // Get padding value (last byte)
    uint8_t padding = data[dataLength - 1];

    // Validate padding
    if (padding == 0 || padding > AES_BLOCK_SIZE) {
        return -1;
    }

    // Verify all padding bytes are correct
    for (int i = dataLength - padding; i < dataLength; i++) {
        if (data[i] != padding) {
            return -1;
        }
    }

    return dataLength - padding;
}


