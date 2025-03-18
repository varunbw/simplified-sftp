#ifndef CRYPTO_SSFTP
#define CRYPTO_SSFTP

#include <vector>
#include <array>
#include "utils.hpp"

namespace Crypto {

    /*
        Normally, SFTP will use asymmetric encryption between the client and server,
        and then negotiate a symmetric encryption key to use for the session.

        In a real-world application, you would use a secure key exchange algorithm
        to negotiate the key and IV.
        For example, you could use Diffie-Hellman key exchange to generate a shared secret,
        and then use that secret to derive the key and IV using a key derivation function (KDF).

        For this implementation, we will use a pre-shared key and IV for AES-256 encryption.
    */

    // Random 32 bytes = 256 bits key
    const std::array<Byte, 32> preSharedKey = {
        0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
        0x76, 0x3b, 0x7b, 0x2e, 0x08, 0x9f, 0x37, 0x67,
        0x83, 0x2d, 0x8a, 0x4f, 0x0e, 0x7d, 0x8d, 0x2d
    };

    // Random 16 bytes = 128 bits IV
    const std::array<Byte, 16> preSharedIV = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };

    /*
        Encrypts the plaintext using AES-256 in CBC mode
        @param plaintext: the plaintext to be encrypted
        @param ciphertext: the encrypted data
        @param key: the encryption key
        @param iv: the initialization vector
        @return true if encryption is successful, false otherwise
    */
    bool EncryptData(const std::vector<Byte>& plaintext, std::vector<Byte>& ciphertext);

    /*
        Decrypts the ciphertext using AES-256 in CBC mode
        @param ciphertext: the ciphertext to be decrypted
        @param plaintext: the decrypted data
        @param key: the decryption key
        @param iv: the initialization vector
        @return true if decryption is successful, false otherwise
    */
    bool DecryptData(const std::vector<Byte>& ciphertext, std::vector<Byte>& plaintext);

    /*
        Calculates the SHA-256 hash of the given data
        @param data: data to be hashed
        @param hash: hash of the data
        @return If operation was successful or not
    */
    bool CalculateHash(const std::vector<Byte>& data, std::vector<Byte>& hash);
};

#endif