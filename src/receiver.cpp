#include <iostream>
#include <fstream>
#include <format>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "../include/crypto.hpp"
#include "../include/logger.hpp"
#include "../include/perf.hpp"
#include "../include/utils.hpp"


class FileReceiver {
private:
    // Socket FD for the sender (client)
    int clientSocket;

    // Server information
    int serverFD;
    sockaddr_in address;
    int addrlen;
    int serverPort;

    /*
        There are three main steps involved in receiving data from the client
        (in this implementation of SFTP)
        1. Read the file size, and file sent by the client
        2. Decrypt the file data (There isn't a function for this, its done in ReceiveFile())
        3. Verify the hash of the decrypted data
        4. Write the decrypted data to a file (No function for this either, its done in ReceiveFile())

        Although these steps can be combined into a single function, they are kept separate
        for better readability and maintainability

        These are private functions since they are only used internally by the class
        and are not meant to be called by the user
    */
    // Step 1
    bool ReadFromClient(std::vector<Byte>& encryptedData);
    // Step 3
    bool ReadAndVerifyHash(std::vector<Byte>& decryptedData);

public:
    FileReceiver(const int port) {

        clientSocket = -1;

        serverFD = -1;
        address = {};
        addrlen = sizeof(address);
        serverPort = port;
     
        return;
    }
    
    ~FileReceiver() {
        CloseConnection();
    }

    bool InitializeServer();
    bool AcceptConnection();
    bool ReceiveFile(const std::string& filename);
    void CloseConnection();
};

/*
    Initialize the server
    @return true if initialization is successful, false otherwise
*/
bool FileReceiver::InitializeServer() {
        
    /*
        Set server address information
        AF_INET: IPv4
        SOCK_STREAM: TCP
    */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serverPort);
    
    // Create socket file descriptor
    serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD == 0) {
        Log::Error("InitializeServer()", "Socket creation error");
        return false;
    }

    // Bind the socket to the port
    int bindStatus = bind(serverFD, (struct sockaddr*)&address, sizeof(address));   
    if (bindStatus < 0) {
        Log::Error("InitializeServer()", "Bind error");
        return false;
    }

    // Start listening for connections
    int listenStatus = listen(serverFD, 3);
    if (listenStatus < 0) {
        Log::Error("InitializeServer()", "Listen error");
        return false;
    }

    Log::Info("InitializeServer()", std::format("Server listening on port {}", serverPort));
    return true;
}

/*
    Accept a connection
    @return true if connection is accepted, false otherwise
*/
bool FileReceiver::AcceptConnection() {
    
    // Accept a connection from the client
    clientSocket = accept(serverFD, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    
    if (clientSocket < 0) {
        Log::Error("AcceptConnection()", "Accept error");
        return false;
    }
    
    return true;
}


/*
    Read the file size, and file sent by the client
    @param encryptedData: vector to store the encrypted data
    @return true if data is read successfully, false otherwise
*/
bool FileReceiver::ReadFromClient(std::vector<Byte>& encryptedData) {

    auto startDuration = TimeNow();
    
    // Read the size of file to be received
    size_t fileSize = -1;
    if (read(clientSocket, &fileSize, sizeof(fileSize)) != sizeof(fileSize)) {
        // This ensures that the file size is read correctly, and is not corrupted
        Log::Error("ReadFromClient()", "Error reading file size");
        return false;
    }

    // Read the file data based on the file size
    size_t bytesRead;
    size_t totalBytesRead = 0;
    std::string buffer(1024, '\0');

    // Main loop to read file data
    while (totalBytesRead < fileSize) {
        /*
            While reading file data, we might accidently read any further data that
            the client will send as well (in this program, we're reading the hash of 
            file later as well).
            To avoid this, we need to calculate how much data to read each iteration, so
            that we don't accidently read more than we're supposed to.

            `bytesToRead` will read either `buffer.size()` data, or the amount of data
            that's yet to be read
        */
        size_t bytesToRead = std::min(buffer.size(), fileSize - totalBytesRead);
        bytesRead = read(clientSocket, &buffer[0], bytesToRead);

        if (bytesRead <= 0) {
            Log::Error("ReadFromClient()", "Error reading file data");
            return false;
        }

        // Insert the `buffer` at `encryptedData.end()`.
        encryptedData.insert(encryptedData.end(), buffer.begin(), buffer.begin() + bytesRead);
        totalBytesRead += bytesRead;
    }

    // Verify that the file data was read correctly
    if (totalBytesRead != fileSize) {
        Log::Error("ReadFromClient()", std::format("File size mismatch, expected {} bytes, but read {} bytes", fileSize, totalBytesRead));
        return false;
    }

    Log::Info("ReadFromClient()", std::format("                  Reading - {}", TimeElapsed(startDuration, TimeNow(), TimePrecision::MILLISECONDS, 1)));
    return true;
}



/*
    Verify the hash of the decrypted data
    @param decryptedData: decrypted data
    @return true if hash is verified successfully, false otherwise
*/
bool FileReceiver::ReadAndVerifyHash(std::vector<Byte>& decryptedData) {

    // Read hash sent by sender
    auto startDuration = TimeNow();
    std::vector<Byte> receivedHash(32);
    int bytesRead = read(clientSocket, receivedHash.data(), receivedHash.size());
    if (bytesRead != receivedHash.size()) {
        Log::Error("ReadAndVerifyHash()", "Error reading hash");
        return false;
    }
    Log::Info("ReadAndVerifyHash()", std::format("          Reading hash - {}", TimeElapsed(startDuration, TimeNow(), TimePrecision::MILLISECONDS, 1)));

    // Calculate hash of decrypted data
    auto hashStart = TimeNow();
    std::vector<Byte> hash = Crypto::CalculateHash(decryptedData);
    if (hash.empty()) {
        Log::Error("ReadAndVerifyHash()", "Error calculating hash");
        return false;
    }
    Log::Info("ReadAndVerifyHash()", std::format("      Calculating hash - {}", TimeElapsed(hashStart, TimeNow(), TimePrecision::MILLISECONDS, 1)));

    // Compare the received hash with the calculated hash
    if (hash != receivedHash) {
        Log::Error("ReadAndVerifyHash()", "Hash mismatch, file contents are invalid");
        return false;
    }

    return true;
}



/*
    Receive a file from the client
    @param filename: path to the file to save
    @return true if file is received successfully, false otherwise
*/
bool FileReceiver::ReceiveFile(const std::string& filename) {

    auto startDuration = TimeNow();
    Log::Info("ReceiveFile()", "Receiving file from client...\n");
    
    std::vector<Byte> encryptedData;
    std::vector<Byte> decryptedData;

    // -- Step 1 --
    // Read the file size, and file sent by the client
    bool readStatus = ReadFromClient(encryptedData);
    if (readStatus == false) {
        Log::Error("ReceiveFile()", "Error reading file sent by client");
        return false;
    }

    // -- Step 2 --
    // Decrypt the Data
    auto decryptionStart = TimeNow();
    bool decryptionStatus = Crypto::DecryptData(encryptedData, decryptedData);
    if (decryptionStatus == false) {
        Log::Error("ReceiveFile()", "Decryption failed");
        return false;
    }
    Log::Info("ReceiveFile()", std::format("                  Decryption - {}", TimeElapsed(decryptionStart, TimeNow(), TimePrecision::MILLISECONDS, 1)));

    // -- Step 3 --
    // Verify the hash of the decrypted data
    bool hashStatus = ReadAndVerifyHash(decryptedData);
    if (hashStatus == false) {
        Log::Error("ReceiveFile()", "Error verifying hash");
        return false;
    }

    // -- Step 4 --
    // Write decrypted Data to file
    auto writeStart = TimeNow();
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        Log::Error("ReceiveFile()", std::format("Failed to create file '{}'", filename));
        return false;
    }
    outfile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
    outfile.close();

    Log::Info("ReceiveFile()", std::format("             Writing to file - {}", TimeElapsed(writeStart, TimeNow(), TimePrecision::MILLISECONDS, 1)));
    Log::Info("ReceiveFile()", std::format("Complete receiving operation - {}\n", TimeElapsed(startDuration, TimeNow(), TimePrecision::MILLISECONDS, 1)));

    Log::Success("ReceiveFile()", std::format("File saved as {} successfully!", filename));
    return true;
}

/*
    Close the connection
*/
void FileReceiver::CloseConnection() {
    
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

int main(int argc, char* argv[]) {

    // if (argc != 2) {
    //     Log::Error("main()", std::format("Usage: {} <file_name_to_save_as>", argv[0]));
    //     return 1;
    // }

    constexpr int port = 8080;
    FileReceiver receiver(port);

    // std::string fileNameToSaveAs = argv[1];

    if (receiver.InitializeServer() == false)
        return 1;
    if (receiver.AcceptConnection() == false)
        return 1;

    if (argc != 2) {
        Log::Error("main()", std::format("Usage: {} <number_of_files>", argv[0]));
        return 1;
    }

    const int numberOfFiles = std::stoi(argv[1]);

    for (int i = 1; i <= numberOfFiles; i++) {
        const std::string fileNameToSaveAs = std::format("../tests/recv/perftest_{}KB.txt", i);
        if (receiver.ReceiveFile(fileNameToSaveAs) == false)
            return 1;
    }


    return 0;
}