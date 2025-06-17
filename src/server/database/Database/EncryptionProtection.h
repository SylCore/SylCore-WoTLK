#ifndef ENCRYPTIONPROTECTION_H
#define ENCRYPTIONPROTECTION_H

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>

#include <stdexcept>
#include <vector>
#include <string>
#include <cstring>



// Auto-generated encryption header — DO NOT EDIT
inline const std::string ENCRYPTION_PREFIX = "YLP_";

inline const uint8_t obfuscatedKey[32] = {
0x2D, 0xF4, 0x62, 0xBD, 0x9D, 0xF0, 0xD1, 0x48, 0x88, 0xE4, 0x99, 0xA2, 0x11, 0x83, 0x99, 0x80, 0x46, 0x26, 0xEB, 0x79, 0x00, 0x6F, 0xBA, 0x58, 0x74, 0xB0, 0xEF, 0xB9, 0x50, 0x5B, 0xF0, 0xDE };

inline const uint8_t obfuscatedIV[16] = {
0x09, 0xC7, 0xDC, 0xD7, 0x7E, 0xF4, 0x17, 0x85, 0x1F, 0x3E, 0x3A, 0xB5, 0x66, 0x51, 0xDE, 0x3A };

inline const uint8_t XORConst1[] = {
0x02, 0xF2 };

inline const uint8_t XORConst2[] = {
0x30, 0x9A };

inline const int XORComplexity = 2;




// Deobfuscation logic.
uint8_t DeobfuscateByte(uint8_t ob, int index);
std::vector<uint8_t> GetDecryptionKey();
std::vector<uint8_t> GetDecryptionIV();



// Check for if the name/value has the prefix.
bool IsEncrypted(const std::string& name);

// Decodes standard Base64 input with no line breaks (as used in .NET's Convert.ToBase64String)
std::vector<unsigned char> Base64DecodeStrict(const std::string& base64);

// Decrypts an AES-256-CBC encrypted Base64 string using hardcoded key and IV.
// Assumes input is a Base64 string produced by .NET's Convert.ToBase64String after AES encryption.
std::string DecryptName(const std::string& base64Ciphertext);

#endif // ENCRYPTIONPROTECTION_H
