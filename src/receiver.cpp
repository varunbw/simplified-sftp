#include <iostream>
#include <fstream>
#include <format>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "../include/crypto.hpp"
#include "../include/logger.hpp"
#include "../include/utils.hpp"

/*
    [IMPORTANT NOTE]
    1. This implementation of SFTP is not a complete implementation of the SFTP protocol.
    2. This is not secure, and should not be used in production.
    3. This is meant for educational purposes only.
*/

/*
    `FileReceiver` is a class to receive files from the sender
    
    Throughout the program, the term "client" is used to refer to the sender, and
    "server" is used to refer to the receiver.

    However, normally, the client AND the server can do both; send and receive files.
    I have not called the class `Server` for this very reason, a client can receive files
    as well. The class `FileSender` is not called `Client` for the same reason.
    
    In this implementation, the client is the sender, and the server is the receiver.
    Get used to it for this program, but remember that this is not the case in a real SFTP.
*/
class FileReceiver {
private:
    // Socket FD for the sender (client)
    int clientSocket;

    // Receiver (server) information
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
        INADDR_ANY: Bind to any address
        htons: Convert port number to network byte order
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
    
    // Read the size of file to be received
    size_t fileSize = -1;
    if (read(clientSocket, &fileSize, sizeof(fileSize)) != sizeof(fileSize)) {
        // This ensures that the file size is read correctly, and is not corrupted
        Log::Error("ReadFromClient()", "Error reading file size");
        return false;
    }

    // Read the file data based on the file size
    int bytesRead;
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

    /*
        The following code is equivalent to the above while loop, but is more readable
        It does not read the file in chunks, instead opting to read it out all at once

        This is not recommended for operation however, as it can lead to memory issues
        Still, if you feel like the above loop is too complicated, you can use this instead
    */
    /* 
        bytesRead = read(clientSocket, &buffer[0], fileSize);
        if (bytesRead < 0) {
            Log::Error("ReadFromClient()", "Error reading file data");
            return false;
        }
        totalBytesRead = bytesRead;
        encryptedData.insert(encryptedData.end(), buffer.begin(), buffer.begin() + bytesRead);
    */ 

    // Verify that the file data was read correctly
    if (totalBytesRead != fileSize) {
        Log::Error("ReadFromClient()", std::format("File size mismatch, expected {} bytes, but read {} bytes", fileSize, totalBytesRead));
        return false;
    }

    return true;
}

/*
    Verify the hash of the decrypted data
    @param decryptedData: decrypted data
    @return true if hash is verified successfully, false otherwise
*/
bool FileReceiver::ReadAndVerifyHash(std::vector<Byte>& decryptedData) {

    // Read hash sent by sender
    std::vector<Byte> receivedHash(32);
    int bytesRead = read(clientSocket, receivedHash.data(), receivedHash.size());
    if (bytesRead != receivedHash.size()) {
        Log::Error("ReadAndVerifyHash()", "Error reading hash");
        return false;
    }

    // Calculate hash of decrypted data
    std::vector<Byte> hash = Crypto::CalculateHash(decryptedData);
    if (hash.empty()) {
        Log::Error("ReadAndVerifyHash()", "Error calculating hash");
        return false;
    }

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
    bool decryptionStatus = Crypto::DecryptData(encryptedData, decryptedData);
    if (decryptionStatus == false) {
        Log::Error("ReceiveFile()", "Decryption failed");
        return false;
    }

    // -- Step 3 --
    // Verify the hash of the decrypted data
    bool hashStatus = ReadAndVerifyHash(decryptedData);
    if (hashStatus == false) {
        Log::Error("ReceiveFile()", "Error verifying hash");
        return false;
    }

    // -- Step 4 --
    // Write decrypted Data to file
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        Log::Error("ReceiveFile()", std::format("Failed to create file '{}'", filename));
        return false;
    }
    outfile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
    outfile.close();

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

    std::cout << std::endl;

    // Check if the correct arguments are passed
    if (argc != 2) {
        Log::Error("main()", std::format("Usage: {} <file_name_to_save_as>", argv[0]));
        return 1;
    }

    constexpr int serverPort = 8080;
    FileReceiver receiver(serverPort);

    std::string fileNameToSaveAs = argv[1];

    if (receiver.InitializeServer() == false)
        return 1;
    if (receiver.AcceptConnection() == false)
        return 1;
    if (receiver.ReceiveFile(fileNameToSaveAs) == false)
        return 1;

    return 0;
}