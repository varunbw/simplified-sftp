#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace Crypto {

    /*
        Struct to hold the encrypted data, including the key and IV
    */
    struct EncryptedData {
        std::vector<unsigned char> key;
        std::vector<unsigned char> iv;
        std::vector<unsigned char> content;
    };

    EncryptedData EncryptFileContents(std::ifstream& infile);
    
};