#include <iostream>
#include <fstream>
#include <format>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "../include/crypto.hpp"
#include "../include/logger.hpp"

// todo move functions outside class definition
class FileReceiver {
private:
    // Socket FD for the sender (client)
    int clientSocket;

    // Server information
    int serverFD;
    sockaddr_in address;
    int addrlen;
    int serverPort;

public:

    FileReceiver(const int port) {

        clientSocket = -1;

        serverFD = -1;
        address = {};
        addrlen = sizeof(address);
        serverPort = port;
     
        return;
    }

    /*
        Initialize the server
        @return true if initialization is successful, false otherwise
    */
    bool InitializeServer() {
        
        // Set server address information
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(serverPort);
        
        // Create socket file descriptor
        if ((serverFD = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            Log::Error("FileReceiver::InitializeServer()", "Socket creation error");
            return false;
        }

        // Bind the socket to the port
        if (bind(serverFD, (struct sockaddr*)&address, sizeof(address)) < 0) {
            Log::Error("FileReceiver::InitializeServer()", "Bind error");
            return false;
        }

        // Start listening for connections
        if (listen(serverFD, 3) < 0) {
            Log::Error("FileReceiver::InitializeServer()", "Listen error");
            return false;
        }

        std::cout << "[INFO] FileReceiver::InitializeServer(): Server listening on port " << serverPort << std::endl;
        return true;
    }

    /*
        Accept a connection
        @return true if connection is accepted, false otherwise
    */
    bool AcceptConnection() {
        
        clientSocket = accept(serverFD, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        
        if (clientSocket < 0) {
            Log::Error("FileReceiver::AcceptConnection()", "Accept error");
            return false;
        }
        
        return true;
    }

    /*
        Receive a file from the client
        @param filename: path to the file to save
        @return true if file is received successfully, false otherwise
    */
    bool ReceiveFile(const std::string& filename) {
        
        std::vector<unsigned char> encryptedData;
        std::vector<unsigned char> decryptedData;

        std::string buffer(1024, '\0');
        int bytesRead;

        // Read the size of file to be received
        size_t fileSize = -1;
        if (read(clientSocket, &fileSize, sizeof(fileSize)) != sizeof(fileSize)) {
            Log::Error("FileReceiver::ReceiveFile()", "Error reading file size");
            return false;
        }

        // Read the file data based on the file size
        size_t totalBytesRead = 0;
        while ((totalBytesRead < fileSize) && (bytesRead = read(clientSocket, &buffer[0], buffer.size())) > 0) {
            encryptedData.insert(encryptedData.end(), buffer.begin(), buffer.begin() + bytesRead);
            totalBytesRead += bytesRead;
        }

        // Verify that the file data was read correctly
        if (totalBytesRead != fileSize) {
            // Log::Error("FileReceiver::ReceiveFile()", "File size mismatch");
            Log::Error("FileReceiver::ReceiveFile()", std::format("File size mismatch, expected {} bytes, but read {} bytes", fileSize, totalBytesRead));
            return false;
        }
        
        // Decrypt the Data
        bool decryptionStatus = Crypto::DecryptData(encryptedData, decryptedData, Crypto::preSharedKey, Crypto::preSharedIV);
        if (decryptionStatus == false) {
            Log::Error("FileReceiver::ReceiveFile()", "Decryption failed");
            return false;
        }

        // Write decrypted Data to file
        std::ofstream outfile(filename, std::ios::binary);
        if (!outfile) {
            Log::Error("FileReceiver::ReceiveFile()", std::format("Failed to create file '{}'", filename));
            return false;
        }

        // Read hash from socket
        std::vector<unsigned char> receivedHash(32);
        if (read(clientSocket, receivedHash.data(), receivedHash.size()) != receivedHash.size()) {
            Log::Error("FileReceiver::ReceiveFile()", "Error reading hash");
            return false;
        }

        // Calculate hash of decrypted data
        std::vector<unsigned char> hash = Crypto::CalculateHash(decryptedData);
        if (hash.empty()) {
            Log::Error("FileReceiver::ReceiveFile()", "Error calculating hash");
            return false;
        }

        // Compare the received hash with the calculated hash
        if (hash != receivedHash) {
            Log::Error("FileReceiver::ReceiveFile()", "Hash mismatch, file contents are invalid");
            return false;
        }

        // Write the decrypted data to the file
        outfile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
        outfile.close();

        Log::Success("FileReceiver::ReceiveFile()", std::format("File saved as {} successfully!", filename));
        return true;
    }

    /*
        Close the connection
    */
    void CloseConnection() {
        
        if (clientSocket != -1) {
            close(clientSocket);
            clientSocket = -1;
        }

        if (serverFD != -1) {
            close(serverFD);
            serverFD = -1;
        }

        return;
    }

    ~FileReceiver() {
        CloseConnection();
    }
};

int main() {

    constexpr int port = 8080;
    FileReceiver receiver(port);

    if (!receiver.InitializeServer())
        return 1;
    if (!receiver.AcceptConnection())
        return 1;
    if (!receiver.ReceiveFile("../temp/received_file.txt"))
        return 1;

    return 0;
}