#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cstring>

#include "../include/crypto.hpp"
#include "../include/logger.hpp"
#include "../include/utils.hpp"

/*
    Big block, ik, but just go through it.

    This file performs the main Encryption and Decryption operations using AES-256 in CBC mode
    We're using the library OpenSSL for this purpose.

    While it looks complicated, its far simpler to understand than implementing our own version
    of AES/S-AES from scratch. The library provides a lot of helper functions to make the process
    easier. There are a lot of lines of code here, but most of them are just error handling.

    The whole program is meant to be modular, meaning that you can put anything you want in the
    encryption and decryption functions, while having to make minor changes anywhere else.
    If you want to experiment with using some other cryptographic algorithms, or maybe try
    some other modes of operation, you can do so by changing the encryption and decryption functions.

    Try putting something as simple as a Caesar cipher in there, and see how it works.
    The only thing you need to keep in mind is that the input and output of the functions should
    remain the same. The input is a vector of bytes, and the output is also a vector of bytes.

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
    /*
        Encrypts the plaintext using AES-256 in CBC mode
        @param plaintext: the plaintext to be encrypted
        @param ciphertext: the encrypted data
        @return true if encryption is successful, false otherwise
    */
    bool EncryptData(const std::vector<Byte>& plaintext, std::vector<Byte>& ciphertext) {

        const std::array<Byte, 32>& key = preSharedKey;
        const std::array<Byte, 16>& iv = preSharedIV;
        
        // Create a new context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (ctx == nullptr) {
            Log::Error("EncryptData()", "Error creating cipher context\n");
            return false;
        }
        
        /*
            Initialize the encryption operation with a cipher type, key, and IV
            Here, we're using AES-256 in CBC mode
            NULL is passed for the cipher type to use the default
        */
        int initStatus = EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data());
        if (initStatus != 1) {
            Log::Error("EncryptData()", "Error initializing encryption operation\n");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // Resize the ciphertext vector to accommodate the encrypted data
        ciphertext.resize(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
        
        /*
            Encrypts the plaintext data
            The ciphertext is written to the ciphertext vector
            The length of the ciphertext is returned in len
        */
        int len;
        int encryptionStatus = EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size());
        if (encryptionStatus != 1) {
            Log::Error("EncryptData()", "Error encrypting data\n");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // Encrypts the "final" data; any data that remains in a partial block. It also writes out the padding.
        int ciphertextLen = len;
        int finalEncryptionStatus = EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
        if (finalEncryptionStatus != 1) {
            Log::Error("EncryptData()", "Error encrypting final data\n");
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
        @return true if decryption is successful, false otherwise
    */
    bool DecryptData(const std::vector<Byte>& ciphertext, std::vector<Byte>& plaintext) {

        const std::array<Byte, 32>& key = preSharedKey;
        const std::array<Byte, 16>& iv = preSharedIV;
        
        // Create a new context
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (ctx == nullptr) {
            Log::Error("DecryptData()", "Error creating cipher context\n");
            return false;
        }
        
        // Initialize the decryption operation with a cipher type, key, and IV
        int initStatus = EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data());
        if (initStatus != 1) {
            Log::Error("DecryptData()", "Error initializing decryption operation\n");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        // Resize the plaintext vector to accommodate the decrypted data
        plaintext.resize(ciphertext.size());

        // Log the sizes of the ciphertext and plaintext
        // Log::Info("DecryptData()", "Ciphertext size: " + std::to_string(ciphertext.size()));
        // Log::Info("DecryptData()", "Plaintext size:  " + std::to_string(plaintext.size()));
        
        /*
            Decrypts the ciphertext data
            The plaintext is written to the plaintext vector
            The length of the plaintext is returned in len
        */
        int len;
        int decryptionStatus = EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size());
        if (decryptionStatus != 1) {
            Log::Error("DecryptData()", "Error decrypting data\n");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }

        
        // Decrypts the "final" data; any data that remains in a partial block. It also writes out the padding.
        int plaintextLen = len;
        int finalDecryptionStatus = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
        if (finalDecryptionStatus != 1) {
            Log::Error("DecryptData()", "Error decrypting final data\n");
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        
        plaintextLen += len;
        plaintext.resize(plaintextLen);
        
        // Free the context
        EVP_CIPHER_CTX_free(ctx);
        return true;
    }

    /*
        Calculates the SHA-256 hash of the given data
        @param data: data to be hashed
        @param hash: hash of the data
        @return If operation was successful or not
    */
    bool CalculateHash(const std::vector<Byte>& data, std::vector<Byte>& hash) {
        // Create a context for the hash operation
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (ctx == nullptr) {
            Log::Error("CalculateHash()", "Error creating hash context");
            return false;
        }

        // Initialize the hash operation with SHA-256
        int initStatus = EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
        if (initStatus != 1) {
            Log::Error("CalculateHash()", "Error initializing hash operation");
            EVP_MD_CTX_free(ctx);
            return false;
        }

        // Provide the data to be hashed
        int updateStatus = EVP_DigestUpdate(ctx, data.data(), data.size());
        if (updateStatus != 1) {
            Log::Error("CalculateHash()", "Error updating hash operation");
            EVP_MD_CTX_free(ctx);
            return false;
        }

        // Finalize the hash operation and retrieve the hash value
        unsigned int hashLen;
        int finalStatus = EVP_DigestFinal_ex(ctx, hash.data(), &hashLen);
        if (finalStatus != 1) {
            Log::Error("CalculateHash()", "Error finalizing hash operation");
            EVP_MD_CTX_free(ctx);
            return false;
        }

        // Free the context
        EVP_MD_CTX_free(ctx);

        return true;
    }


    /*
        Below are functions implemention one of the most basic encryption algorithms, the Caesar cipher.
        The Caesar cipher is a substitution cipher where each letter in the plaintext is shifted by a
        fixed number of positions in the alphabet. In this case, we're using a shift of 3.
        This is just an example to show how you can implement your own encryption and decryption functions.
        You can replace these functions with any other encryption algorithm you want.
        The EncryptData and DecryptData functions are the same as the ones above, but they use the Caesar cipher
        instead of AES-256.

        Note that this is not a secure encryption method and should not be used for any real-world applications.
    */
    /*
    bool EncryptData(const std::vector<Byte>& plaintext, std::vector<Byte>& ciphertext) {

        const int shift = 3; // Shift value for Caesar cipher
        ciphertext.resize(plaintext.size());
        for (size_t i = 0; i < plaintext.size(); ++i) {
            ciphertext[i] = plaintext[i] + shift;
        }

        return true;
    }
    
    bool DecryptData(const std::vector<Byte>& ciphertext, std::vector<Byte>& plaintext) {

        const int shift = 3; // Shift value for Caesar cipher
        plaintext.resize(ciphertext.size());
        for (size_t i = 0; i < ciphertext.size(); ++i) {
            plaintext[i] = ciphertext[i] - shift;
        }

        return true;
    }
    */

    /*
        Basic hash function that calculates a simple hash of the data
        This is not a secure hash function and should not be used for any real-world applications
        It's just an example to show how you can implement your own hash function

        @param data: the data to be hashed
        @param hashResult: the resulting hash
        @return true (yes, its that simple) :)
    */
    /*
    bool BasicHashFunction(const std::vector<Byte>& data, std::vector<Byte>& hashResult) {

        // Initialize the hash result with 8 bytes, and fill them with 0s
        hashResult.resize(8);
        fill(hashResult.begin(), hashResult.end(), 0);

        // Do a simple hash calculation by adding the bytes together in a cyclic fashion
        // Groups of 8 bytes are added together to form the hash
        int hashIdx = 0;
        for (Byte byte : data) {
            hashResult[hashIdx++] += byte;
            hashIdx %= 8;
        }

        return true;
    }
    */
};