#include <iostream>
#include <fstream>
#include <format>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
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
    `FileSender` is a class to send files to the receiver
    
    Throughout the program, the term "client" is used to refer to the sender, and
    "server" is used to refer to the receiver.

    However, normally, the client AND the server can do both; send and receive files.
    I have not called the class `Client` for this very reason, a server can send files
    as well. The class `FileReceiver` is not called `Server` for the same reason.
    
    In this implementation, the client is the sender, and the server is the receiver.
    Get used to it for this program, but remember that this is not the case in a real SFTP.
*/

class FileSender {
private:
    // Sender (client) socket
    int socketFD;

    // Receiver (server) address information
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
        Log::Error("ConnectToServer()", "Socket creation error");
        return false;
    }

    // Convert addresses from text to binary form
    int inetStatus = inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    if (inetStatus <= 0) {
        Log::Error("ConnectToServer()", "Invalid address/Address not supported");
        return false;
    }

    // Connect to the server
    int connectStatus = connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (connectStatus < 0) {
        Log::Error("ConnectToServer()", "Connection failed");
        return false;
    }

    return true;
}

/*
    Load the contents of a file into a vector
    @param filename: path to the file
    @param data: vector to store the file contents
    @return true if file is loaded successfully, false otherwise

    Normally, the file would be read in chunks and sent to the server,
    to avoid creating unnecessarily large buffers and wasting memory.
    But for simplicity, the entire file is read into memory at once.
*/
bool FileSender::LoadFileIntoVector(const std::string& filename, std::vector<Byte>& data) {
    // Open file in binary mode
    std::ifstream infile(filename, std::ios::binary);
    if (infile.fail()) {
        Log::Error("LoadFileIntoVector()", std::format("Failed to open file '{}'", filename));
        return false;
    }

    // Load the file contents into a vector
    // istreambuf_iterator reads the file contents as a stream of bytes
    data = std::vector<Byte>((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

    return true;
}


/*
    Encrypt the file contents and send it to the server
    @param plainFileData: file contents
    @return true if file is sent successfully, false otherwise

    Similar to LoadFileIntoVector(), the chunks that were read into memory
    could be encrypted and sent to the server in chunks, to avoid creating
    unnecessarily large buffers and wasting memory.

    But for simplicity, the entire file is encrypted and sent at once.
*/
bool FileSender::EncryptAndSend(const std::vector<Byte>& plainFileData) {

    /*
        Encrypt the file contents using AES-256-CBC encryption
        See Crypto::EncryptData() in crypto.cpp for more details
        
        In a normal implementation of SFTP, the file would be encrypted using the server's public key
        and decrypted using the server's private key.
        However, for simplicity, a pre-shared key is used here along with a pre-shared IV (Initialization Vector)
        See Crypto::EncryptData() in crypto.cpp for more details
        The pre-shared key and IV are hardcoded in crypto.hpp

        You can use any 256 bit key and 128 bit IV for encryption
    */
    std::vector<Byte> encryptedData;
    bool encryptionStatus = Crypto::EncryptData(plainFileData, encryptedData);
    if (encryptionStatus == false) {
        Log::Error("EncryptAndSend()", "Error encrypting file");
        return false;
    }

    /*
        Send size of encrypted data to the server
        This is done so that the server knows how much data to expect

        We cannot send the size of the file prior to encryption, since AES encryption
        will change the size of the data (padding will be added)
        Hence, encryption is carried first, then the size of the vector is taken
    */
    size_t fileSize = encryptedData.size();
    int bytesSent = send(socketFD, &fileSize, sizeof(fileSize), 0);
    if (bytesSent < 0) {
        Log::Error("EncryptAndSend()", "Error sending file size");
        return false;
    }
    
    // Send the encrypted file contents to the server in 1024 byte chunks
    size_t totalBytesSent = 0;
    size_t totalSize = encryptedData.size();
    while (totalBytesSent < totalSize) {
        // Send the min amongst 1024 bytes, or how much ever is left to send
        size_t chunkSize = std::min(static_cast<size_t>(32768), totalSize - totalBytesSent);

        int sentBytes = send(socketFD, encryptedData.data() + totalBytesSent, chunkSize, 0);
        if (sentBytes < 0) {
            Log::Error("EncryptAndSend()", "Error sending encrypted file");
            return false;
        }

        totalBytesSent += chunkSize;
    }

    /*
        The following code is equivalent to the above while loop, but is more readable
        It does not send the file in chunks, instead opting to send it out all at once

        This is not recommended for operation however, as it can lead to memory issues

        Still, if you feel like the above loop is too complicated, you can use this instead
    */
    /* 
        int sentBytes = send(socketFD, encryptedData.data(), encryptedData.size(), 0);
        if (sentBytes < 0) {
            Log::Error("EncryptAndSend()", "Error sending encrypted file");
            return false;
        }
    */

    return true;
}


/*
    Calculate hash of the file and send it to the server
    @param data: file contents
    @return true if hash is sent successfully, false otherwise
*/
bool FileSender::CalculateHashAndSend(const std::vector<Byte>& data) {

    // Calculate hash of the file
    std::vector<Byte> hash(32, 0);
    bool hashStatus = Crypto::CalculateHash(data, hash);
    if (hashStatus == false) {
        Log::Error("CalculateHashAndSend()", "Error calculating hash");
        return false;
    }

    /*
        Normally, you'd also encrypt the hash of the file using `Crypto::EncryptData()`
        and send it to the server.
        But for simplicity, the hash is sent as is.
    */
    int sentBytes = send(socketFD, &hash[0], hash.size(), 0);
    if (sentBytes < 0) {
        Log::Error("CalculateHashAndSend()", "Error sending hash");
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
        Log::Error("SendFile()", "Error loading file");
        return false;
    }
    
    // -- Step 2 --
    // Encrypt and send the file to the server
    bool sentToServer = EncryptAndSend(plainFileData);
    if (sentToServer == false) {
        Log::Error("SendFile()", "Error sending encrypted file");
        return false;
    }
    
    // -- Step 3 --
    // Calculate hash and send it to the server
    bool sentHash = CalculateHashAndSend(plainFileData);
    if (sentHash == false) {
        Log::Error("SendFile()", "Error sending hash");
        return false;
    }

    Log::Success("SendFile()", std::format("File {} sent successfully!", filename));
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


int main(int argc, char* argv[]) {

    std::cout << std::endl;

    // Configuration
    const std::string serverIP("127.0.0.1");
    constexpr int serverPort = 8080;

    // Handle flags
    if (argc < 3) {
        Log::Error("main()", std::format("Usage: {} [-f <filename>] [-n <number_of_files>]", argv[0]));
        return -1;
    }

    // Connect to the server
    FileSender sender(serverIP, serverPort);
    if (sender.ConnectToServer() == false)
        return 1;

    std::string flag = argv[1];

    /*
        The client is now ready to send files, and the
        "configration" is done

        After this point, all the code is just meant to show you
        how to send files, and is not part of the protocol per se

        The only line you should know is
        `sender.SendFile(filename)`
        which is the function that, well..., sends the file (ik, shocker again)

        The rest of the code is just to show you how to use the class,
        and you can just skip straight to the SendFile() function
        if you want to
    */

    /*
        argv[1] = -f
        argv[2] = file name

        The user wants to send a single file, and not a number of files
    */
    if (flag == "-f") {
        if (argc < 3) {
            Log::Error("main()", "Missing file name after -f");
            return -1;
        }

        std::string fileToSend = argv[2];
        if (sender.SendFile(fileToSend) == false)
            return 1;
        return 0;
    }

    /*
        For the sake of understanding, you should first try the above option
        which is sending a single file
        Then, try this option which is sending multiple files

        There is no need for you to learn how to send multiple files, and you
        can just stop here
        But if you want to learn how to send multiple files, then carry on
    */
    /*
        argv[1] = -n
        argv[2] = number of files

        The user wants to send a number of files, and not just one file
        The files are named `perftest_<number>KB.txt`, where <number> is the
        number of files to send
        The files are located in the `tests/send` directory
    */
    else if (flag == "-n") {
        if (argc < 3) {
            Log::Error("main()", "Missing number of files after -n");
            return -1;
        }

        int numberOfFiles = 0;
        try {
            numberOfFiles = std::stoi(argv[2]);
        }
        catch (std::invalid_argument&) {
            Log::Error("main()", "Invalid number of files");
            return -1;
        }

        for (int j = 1; j <= numberOfFiles; j++) {
            const std::string fileToSend = std::format("tests/send/perftest_{}KB.txt", j);
            if (sender.SendFile(fileToSend) == false)
                return 1;
        }
        return 0;
    }
    /*
        The user has provided an invalid flag
        Print usage and exit
    */
    else {
        Log::Error("main()", std::format("Unknown flag {}", flag));
        return -1;
    }

    return 0;
}