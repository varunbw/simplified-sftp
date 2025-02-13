#include <iostream>
#include <fstream>
#include <format>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

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
            std::cerr << "[ERROR] FileSender::ConnectToServer(): Socket creation error\n";
            return false;
        }

        if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "[ERROR] FileSender::ConnectToServer(): Invalid address/Address not supported\n";
            return false;
        }

        if (connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "[ERROR] FileSender::ConnectToServer(): Connection failed\n";
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
        if (!infile) {
            std::cerr << "[ERROR] FileSender::SendFile(): Failed to open file '" << filename << "'\n";
            return false;
        }

        // Read file in chunks and send it to the server
        std::string buffer(1024, '\0');
        while (infile.read(&buffer[0], buffer.size()) || infile.gcount() > 0) {
            if (send(socketFD, buffer.c_str(), infile.gcount(), 0) < 0) {
                std::cerr << "[ERROR] FileSender::SendFile(): Error sending file\n";
                return false;
            }
        }

        // std::cout << "[INFO] FileSender::SendFile(): File sent successfully!\n";
        std::cout << std::format("[INFO] FileSender::SendFile(): File {} sent successfully!\n", filename);
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
