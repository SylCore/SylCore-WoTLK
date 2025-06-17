#include "EncryptionProtection.h"


bool IsEncrypted(const std::string& name) {
    if (name.starts_with(ENCRYPTION_PREFIX))
        return true;

    return false;
}


// Deobfuscation logic
uint8_t DeobfuscateByte(uint8_t ob, int index) {
    uint8_t result = ob;
    for (int layer = XORComplexity - 1; layer >= 0; --layer) {
        uint8_t xor1 = XORConst1[layer];
        uint8_t xor2 = XORConst2[layer];
        uint8_t rotated = (result >> 3) | (result << 5);
        result = rotated ^ ((xor2 + static_cast<uint8_t>(index * 13) + layer) ^ xor1);
    }
    return result;
}



/* USE THIS IF THE NEW MODULAR ENCRYPTION DIDNT WORK!
uint8_t DeobfuscateByte(uint8_t ob, int index) {
    // Reverse rotation: right by 3 bits (undoes C#'s left rotation)
    uint8_t rotated = (ob >> 3) | (ob << 5);

    // Compute XOR value (same as C# calculation)
    uint8_t xorValue = (XOR_CONST2 + static_cast<uint8_t>(index * 13)) ^ XOR_CONST1;

    return rotated ^ xorValue;
}
*/

// Get decrypted key
std::vector<uint8_t> GetDecryptionKey() {
    std::vector<uint8_t> decrypted;
    for (size_t i = 0; i < sizeof(obfuscatedKey); i++) {
        decrypted.push_back(DeobfuscateByte(obfuscatedKey[i], static_cast<int>(i)));
    }
    return decrypted;
}

// Get decrypted IV
std::vector<uint8_t> GetDecryptionIV() {
    std::vector<uint8_t> decrypted;
    for (size_t i = 0; i < sizeof(obfuscatedIV); i++) {
        decrypted.push_back(DeobfuscateByte(obfuscatedIV[i], static_cast<int>(i)));
    }
    return decrypted;
}

std::vector<unsigned char> Base64DecodeStrict(const std::string& base64)
{
    BIO* bio, * b64;
    int decodeLen = ((base64.length() * 3) / 4);
    std::vector<unsigned char> buffer(decodeLen);

    bio = BIO_new_mem_buf(base64.data(), static_cast<int>(base64.length()));
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    int len = BIO_read(bio, buffer.data(), base64.length());
    if (len < 0)
        throw std::runtime_error("Failed to decode Base64");

    buffer.resize(len);
    BIO_free_all(bio);
    return buffer;
}


std::string DecryptName(const std::string& base64Ciphertext)
{
    if (!base64Ciphertext.starts_with(ENCRYPTION_PREFIX)) {
        return base64Ciphertext;
    }

    auto key = GetDecryptionKey();
    auto iv = GetDecryptionIV();


    // Removes the prefix first.
    std::vector<unsigned char> cipher_bytes = Base64DecodeStrict(base64Ciphertext.substr(ENCRYPTION_PREFIX.size()));

    std::vector<unsigned char> decrypted(cipher_bytes.size() + AES_BLOCK_SIZE);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) throw std::runtime_error("Failed to create EVP context");

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()))
        throw std::runtime_error("EVP_DecryptInit_ex failed");

    int len = 0;
    int plaintext_len = 0;

    if (1 != EVP_DecryptUpdate(ctx, decrypted.data(), &len, cipher_bytes.data(), cipher_bytes.size()))
        throw std::runtime_error("EVP_DecryptUpdate failed");

    plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, decrypted.data() + len, &len))
        throw std::runtime_error("EVP_DecryptFinal_ex failed");

    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return std::string(decrypted.begin(), decrypted.begin() + plaintext_len);
}

