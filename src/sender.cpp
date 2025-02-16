#include <iostream>
#include <fstream>
#include <format>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../include/crypto.hpp"
#include "../include/logger.hpp"

// todo move functions outside class definition
class FileSender {
private:
    // Sender (client) socket
    int socketFD;

    // Server address information
    sockaddr_in serverAddr;
    std::string serverIP;
    int serverPort;

public:
    FileSender(const std::string& ip, const int port) {

        socketFD = -1;

        serverAddr = {};
        serverIP = ip;
        serverPort = port;

        return;
    }

    /*
        Connect to the server
        @return true if connection is successful, false otherwise
    */
    bool ConnectToServer() {
        
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
        Send a file to the server
        @param filename: path to the file to send
        @return true if file is sent successfully, false otherwise
    */
    bool SendFile(const std::string& filename) {
        
        // Open file in binary mode
        std::ifstream infile(filename, std::ios::binary);
        if (infile.fail()) {
            Log::Error("FileSender::SendFile()", std::format("Failed to open file '{}'", filename));
            return false;
        }

        // Load the file contents into a vector
        std::vector<unsigned char> plainFileData((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

        // Encrypt the file contents
        std::vector<unsigned char> encryptedData;
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

        // todo move hash sending to another function
        // Calculate hash of the file
        std::vector<unsigned char> hash = Crypto::CalculateHash(plainFileData);
        if (hash.empty()) {
            Log::Error("FileSender::SendFile()", "Error calculating hash");
            return false;
        }

        // Send hash data to server
        if (send(socketFD, hash.data(), hash.size(), 0) < 0) {
            Log::Error("FileSender::SendFile()", "Error sending hash");
            return false;
        }

        Log::Success("FileSender::SendFile()", std::format("File {} sent successfully!", filename));
        return true;
    }

    /*
        Close the connection
    */
    void CloseConnection() {

        if (socketFD != -1) {
            close(socketFD);
            socketFD = -1;
        }

        return;
    }

    ~FileSender() {
        CloseConnection();
    }
};


int main(void) {

    const std::string IP("127.0.0.1");
    constexpr int port = 8080;

    FileSender sender(IP, port);

    if (!sender.ConnectToServer())
        return 1;
    if (!sender.SendFile("../temp/file_to_send.txt"))
        return 1;

    return 0;
}
