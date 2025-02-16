#ifndef CRYPTO_SSFTP
#define CRYPTO_SSFTP

#include <vector>

namespace Crypto {

    // 32 bytes = 256 bits key
    const std::vector<unsigned char> preSharedKey = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
        0x76, 0x3b, 0x7b, 0x2e, 0x08, 0x9f, 0x37, 0x67,
        0x83, 0x2d, 0x8a, 0x4f, 0x0e, 0x7d, 0x8d, 0x2d
    };

    // 16 bytes = 128 bits IV
    const std::vector<unsigned char> preSharedIV = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };

    bool EncryptData(
        const std::vector<unsigned char>& plaintext,
        std::vector<unsigned char>& ciphertext,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& iv
    );

    bool DecryptData(
        const std::vector<unsigned char>& ciphertext,
        std::vector<unsigned char>& plaintext,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& iv
    );

    std::vector<unsigned char> CalculateHash(const std::vector<unsigned char>& data);
};

#endif