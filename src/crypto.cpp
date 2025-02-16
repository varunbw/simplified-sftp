#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include "../include/crypto.hpp"
#include "../include/logger.hpp"

/*
    This file performs the main Encryption and Decryption operations using AES-256 in CBC mode
    We're using the library OpenSSL for this purpose. If needed, you can look up some basic
    tutorials on how to use OpenSSL for encryption and decryption.

    While it looks complicated, its far simpler to understand than implementing our own version
    of AES/S-AES from scratch. The library provides a lot of helper functions to make the process
    easier.

    The whole program is meant to be modular, meaning that you can put anything you want in the
    encryption and decryption functions, while having to make minor changes anywhere else.
    If you want to experiment with using some other cryptographic algorithms, or maybe try
    some other modes of operation, you can do so by changing the encryption and decryption functions.

    If you change the function signature (the parameters and return type), you'll have to make
    changes in the header file and where you call the function accordingly. The rest of the program
    will remain the same.
    
    The library provides a lot of cryptographic functions and algorithms. We're using the EVP
    (Envelope) interface here.
    
    The main functions used here are:
    - EVP_CIPHER_CTX_new(): Creates a new cipher context
    - EVP_EncryptInit_ex(): Initializes the encryption operation
    - EVP_EncryptUpdate(): Encrypts the data
    - EVP_EncryptFinal_ex(): Encrypts the final data
    - EVP_CIPHER_CTX_free(): Frees the cipher context
    - EVP_DecryptInit_ex(): Initializes the decryption operation
    - EVP_DecryptUpdate(): Decrypts the data
    - EVP_DecryptFinal_ex(): Decrypts the final data
*/


namespace Crypto {

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

    /*
        Encrypts the plaintext using AES-256 in CBC mode
        @param plaintext: the plaintext to be encrypted
        @param ciphertext: the encrypted data
        @param key: the encryption key
        @param iv: the initialization vector
        @return true if encryption is successful, false otherwise
    */
    bool EncryptData(
        const std::vector<unsigned char>& plaintext,
        std::vector<unsigned char>& ciphertext,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& iv
    ) {
        
        // Create a new context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        return false;
        
        // Initialize the encryption operation with a cipher type, key, and IV
        int initStatus = EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data());
        if (initStatus != 1) {
            std::cerr << "[ERROR] Crypto::EncryptData(): Error initializing encryption operation\n";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // Resize the ciphertext vector to accommodate the encrypted data
        ciphertext.resize(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
        
        // Encrypt the plaintext
        int len;
        int encryptionStatus = EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size());
        if (encryptionStatus != 1) {
            std::cerr << "[ERROR] Crypto::EncryptData(): Error encrypting data\n";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // Encrypts the "final" data; any data that remains in a partial block. It also writes out the padding.
        int ciphertextLen = len;
        int finalEncryptionStatus = EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
        if (finalEncryptionStatus != 1) {
            std::cerr << "[ERROR] Crypto::EncryptData(): Error encrypting final data\n";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        ciphertextLen += len;
        ciphertext.resize(ciphertextLen);
        
        // Free the context
        EVP_CIPHER_CTX_free(ctx);
        return true;
    }
    
    
    /*
        Decrypts the ciphertext using AES-256 in CBC mode
        @param ciphertext: the ciphertext to be decrypted
        @param plaintext: the decrypted data
        @param key: the decryption key
        @param iv: the initialization vector
        @return true if decryption is successful, false otherwise
    */
    bool DecryptData(
        const std::vector<unsigned char>& ciphertext,
        std::vector<unsigned char>& plaintext,
        const std::vector<unsigned char>& key,
        const std::vector<unsigned char>& iv
    ) {
        
        // Create a new context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            std::cerr << "[ERROR] Crypto::DecryptData(): Error creating cipher context\n";
            return false;
        }
        
        // Initialize the decryption operation with a cipher type, key, and IV
        int initStatus = EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data());
        if (initStatus != 1) {
            std::cerr << "[ERROR] Crypto::DecryptData(): Error initializing decryption operation\n";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // Resize the plaintext vector to accommodate the decrypted data
        plaintext.resize(ciphertext.size());
        std::cout << "Ciphertext size: " << ciphertext.size() << std::endl;
        std::cout << "Plaintext size: " << plaintext.size() << std::endl;
        
        // Decrypt the ciphertext
        int len;
        int decryptionStatus = EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size());
        if (decryptionStatus != 1) {
            std::cerr << "[ERROR] Crypto::DecryptData(): Error decrypting data\n";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        
        int plaintextLen = len;
        // Decrypts the "final" data; any data that remains in a partial block. It also writes out the padding.
        int finalDecryptionStatus = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
        if (finalDecryptionStatus != 1) {
            std::cerr << "[ERROR] Crypto::DecryptData(): Error decrypting final data\n";
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        plaintextLen += len;
        plaintext.resize(plaintextLen);
        
        // Free the context
        EVP_CIPHER_CTX_free(ctx);
        return true;
    }

    std::vector<unsigned char> CalculateHash(const std::vector<unsigned char>& data) {
        // Create a context for the hash operation
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            Log::Error("Crypto::CalculateHash", "Error creating hash context");
            return {};
        }

        // Initialize the hash operation with SHA-256
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
            Log::Error("Crypto::CalculateHash", "Error initializing hash operation");
            EVP_MD_CTX_free(ctx);
            return {};
        }

        // Provide the data to be hashed
        if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
            Log::Error("Crypto::CalculateHash", "Error updating hash operation");
            EVP_MD_CTX_free(ctx);
            return {};
        }

        // Finalize the hash operation and retrieve the hash value
        std::vector<unsigned char> hash(EVP_MD_size(EVP_sha256()));
        unsigned int hashLen;
        if (EVP_DigestFinal_ex(ctx, hash.data(), &hashLen) != 1) {
            Log::Error("Crypto::CalculateHash", "Error finalizing hash operation");
            EVP_MD_CTX_free(ctx);
            return {};
        }

        // Free the context
        EVP_MD_CTX_free(ctx);

        return hash;
    }
    
    
};