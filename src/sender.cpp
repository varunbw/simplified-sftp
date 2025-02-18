#include <iostream>
#include <fstream>
#include <format>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../include/main.hpp"

class FileSender {
private:
    // Sender (client) socket
    int socketFD;

    // Server address information
    sockaddr_in serverAddr;
    std::string serverIP;
    int serverPort;

    /*
        There are three main steps involved in sending data to the server
        (in this implementation of SFTP)
        1. Load the file contents into a vector
        2. Encrypt the file contents and send it to the server
        3. Calculate hash of the file and send it to the server

        Although these steps can be combined into a single function, they are kept separate
        for better readability and maintainability

        These are private functions since they are only used internally by the class
        and are not meant to be called by the user
    */
    // Step 1
    bool LoadFileIntoVector(const std::string& filename, std::vector<Byte>& data);
    // Step 2
    bool EncryptAndSend(const std::vector<Byte>& data);
    // Step 3
    bool CalculateHashAndSend(const std::vector<Byte>& data);

public:
    FileSender(const std::string& ip, const int port) {

        socketFD = -1;

        serverAddr = {};
        serverIP = ip;
        serverPort = port;

        return;
    }

    ~FileSender() {
        CloseConnection();
    }

    bool ConnectToServer();
    bool SendFile(const std::string& fileName);
    void CloseConnection();
};


/*
    Connect to the server
    @return true if connection is successful, false otherwise
*/
bool FileSender::ConnectToServer() {
        
    // Set server information
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);

    // Create a socket - IPv4, TCP
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        Log::Error("FileSender::ConnectToServer()", "Socket creation error");
        return false;
    }

    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        Log::Error("FileSender::ConnectToServer()", "Invalid address/Address not supported");
        return false;
    }

    if (connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        Log::Error("FileSender::ConnectToServer()", "Connection failed");
        return false;
    }

    return true;
}

/*
    Load the contents of a file into a vector
    @param filename: path to the file
    @param data: vector to store the file contents
    @return true if file is loaded successfully, false otherwise
*/
bool FileSender::LoadFileIntoVector(const std::string& filename, std::vector<Byte>& data) {
    // Open file in binary mode
    std::ifstream infile(filename, std::ios::binary);
    if (infile.fail()) {
        Log::Error("FileSender::SendFile()", std::format("Failed to open file '{}'", filename));
        return false;
    }

    // Load the file contents into a vector
    data = std::vector<Byte>((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

    return true;
}


/*
    Encrypt the file contents and send it to the server
    @param plainFileData: file contents
    @return true if file is sent successfully, false otherwise
*/
bool FileSender::EncryptAndSend(const std::vector<Byte>& plainFileData) {

    // Encrypt the file contents
    std::vector<Byte> encryptedData;
    bool encryptionStatus = Crypto::EncryptData(plainFileData, encryptedData, Crypto::preSharedKey, Crypto::preSharedIV);
    if (encryptionStatus == false) {
        Log::Error("FileSender::SendFile()", "Error encrypting file");
        return false;
    }

    // Send size of file to server, so it knows how much data to expect
    size_t fileSize = encryptedData.size();
    if (send(socketFD, &fileSize, sizeof(fileSize), 0) < 0) {
        Log::Error("FileSender::SendFile()", "Error sending file size");
        return false;
    }
    
    // Send the encrypted file contents to the server in 1024 byte chunks
    size_t sentSize = 0;
    size_t totalSize = encryptedData.size();
    while (sentSize < totalSize) {
        // Send the min amongst 1024 bytes, or how much ever is left to send
        size_t chunkSize = std::min(static_cast<size_t>(1024), totalSize - sentSize);

        if (send(socketFD, encryptedData.data() + sentSize, chunkSize, 0) < 0) {
            Log::Error("FileSender::SendFile()", "Error sending encrypted file");
            return false;
        }

        sentSize += chunkSize;
    }

    return true;
}


/*
    Calculate hash of the file and send it to the server
    @param data: file contents
    @return true if hash is sent successfully, false otherwise
*/
bool FileSender::CalculateHashAndSend(const std::vector<Byte>& data) {

    // Calculate hash of the file
    std::vector<Byte> hash = Crypto::CalculateHash(data);
    if (hash.empty()) {
        Log::Error("FileSender::SendFile()", "Error calculating hash");
        return false;
    }

    // Send hash data to server
    if (send(socketFD, hash.data(), hash.size(), 0) < 0) {
        Log::Error("FileSender::SendFile()", "Error sending hash");
        return false;
    }

    return true;
}


/*
    Send a file to the server
    @param filename: path to the file to send
    @return true if file is sent successfully, false otherwise
*/
bool FileSender::SendFile(const std::string& filename) {
    
    // -- Step 1 --
    // Load the file into a vector
    std::vector<Byte> plainFileData;
    bool loadedData = LoadFileIntoVector(filename, plainFileData);
    if (loadedData == false) {
        Log::Error("FileSender::SendFile()", "Error loading file");
        return false;
    }
    
    // -- Step 2 --
    // Encrypt and send the file to the server
    bool sentToServer = EncryptAndSend(plainFileData);
    if (sentToServer == false) {
        Log::Error("FileSender::SendFile()", "Error sending encrypted file");
        return false;
    }
    
    // -- Step 3 --
    // Calculate hash and send it to the server
    bool sentHash = CalculateHashAndSend(plainFileData);
    if (sentHash == false) {
        Log::Error("FileSender::SendFile()", "Error sending hash");
        return false;
    }

    Log::Success("FileSender::SendFile()", std::format("File {} sent successfully!", filename));
    return true;
}

/*
    Close the connection
*/
void FileSender::CloseConnection() {

    if (socketFD != -1) {
        close(socketFD);
        socketFD = -1;
    }

    return;
}


int main(void) {

    const std::string IP("127.0.0.1");
    constexpr int port = 8080;

    FileSender sender(IP, port);

    if (sender.ConnectToServer() == false)
        return 1;
    if (sender.SendFile("../temp/file_to_send.txt") == false)
        return 1;

    return 0;
}
