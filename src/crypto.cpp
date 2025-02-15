#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "../include/crypto.hpp"

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

    std::vector<unsigned char> EncryptFileContents(std::ifstream& infile);

    /*
        Encrypts the contents of a file using AES-256 in CBC mode
        @param infile: the input file stream
        @return the encrypted data
    */
    std::vector<unsigned char> EncryptFileContents(std::ifstream& infile) {
        
        // Read file contents into a vector
        std::vector<unsigned char> fileContents((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        
        // Encrypt the file contents
        std::vector<unsigned char> encryptedContents;
        if (!EncryptData(fileContents, encryptedContents, preSharedKey, preSharedIV)) {
            std::cerr << "[ERROR] Crypto::EncryptFileContents(): Error encrypting file\n";
            return {};
        }
        
        return encryptedContents;
    }

    /*
        Decrypts the contents of a file using AES-256 in CBC mode
        @param infile: the input file stream
    */
    bool DecryptFileContents(std::vector<unsigned char>& encryptedContent, std::vector<unsigned char>& decryptedContent) {
        
        // Decrypt the file contents
        if (DecryptData(encryptedContent, decryptedContent, preSharedKey, preSharedIV) == false) {
            std::cerr << "[ERROR] Crypto::DecryptFileContents(): Error decrypting file\n";
            return false;
        }

        return true;
    }

    
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
    
    /*
        int main() {
            std::vector<unsigned char> key(32); // 256-bit key
            std::vector<unsigned char> iv(16);  // 128-bit IV
            
            // Generate random key and IV
            RAND_bytes(key.data(), key.size());
            RAND_bytes(iv.data(), iv.size());
            
            std::string plaintext = "Hello, OpenSSL!";
            std::vector<unsigned char> plaintext_vec(plaintext.begin(), plaintext.end());
            std::vector<unsigned char> ciphertext;
            std::vector<unsigned char> decryptedtext;
            
            if (EncryptData(plaintext_vec, ciphertext, key, iv)) {
                std::cout << "Encryption successful!" << std::endl;
            } else {
                std::cerr << "Encryption failed!" << std::endl;
            }
            
            if (DecryptData(ciphertext, decryptedtext, key, iv)) {
                std::cout << "Decryption successful!" << std::endl;
                std::string decrypted_str(decryptedtext.begin(), decryptedtext.end());
                std::cout << "Decrypted text: " << decrypted_str << std::endl;
            } else {
                std::cerr << "Decryption failed!" << std::endl;
            }
            
            return 0;
        }
    */
};