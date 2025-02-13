#include <iostream>
#include <fstream>
#include <format>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

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

        serverPort = port;
        serverFD = -1;
        clientSocket = -1;
        addrlen = sizeof(address);
        address = {};
     
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
            std::cerr << "[ERROR] FileReceiver::InitializeServer(): Socket creation error\n";
            return false;
        }

        // Bind the socket to the port
        if (bind(serverFD, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "[ERROR] FileReceiver::InitializeServer(): Bind error\n";
            return false;
        }

        // Start listening for connections
        if (listen(serverFD, 3) < 0) {
            std::cerr << "[ERROR] FileReceiver::InitializeServer(): Listen error\n";
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
            std::cerr << "[ERROR] FileReceiver::AcceptConnection(): Accept error\n";
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
        
        // Open file to write received data
        std::ofstream outfile(filename, std::ios::binary);

        if (!outfile) {
            std::cerr << "[ERROR] FileReceiver::ReceiveFile(): Failed to create file '" << filename << "'\n";
            close(clientSocket);
            return false;
        }

        std::string buffer(1024, '\0');
        int bytesRead;

        // Read data from client and write to file
        while ((bytesRead = read(clientSocket, &buffer[0], buffer.size())) > 0) {
            outfile.write(buffer.c_str(), bytesRead);
        }

        std::cout << std::format("[INFO] FileSender::SendFile(): File saved as {} successfully!\n", filename);

        outfile.close();
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