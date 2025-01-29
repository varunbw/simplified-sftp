#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class FileSender {
private:
    int sock;
    struct sockaddr_in serv_addr;
    std::string server_ip;
    int port;

public:
    FileSender(const std::string& ip, int port) : server_ip(ip), port(port), sock(-1) {}

    bool ConnectToServer() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Socket creation error\n";
            return false;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address/Address not supported\n";
            return false;
        }

        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Connection failed\n";
            return false;
        }

        return true;
    }

    bool SendFile(const std::string& filename) {
        std::ifstream infile(filename, std::ios::binary);
        if (!infile) {
            std::cerr << "Failed to open file '" << filename << "'\n";
            return false;
        }

        char buffer[1024];
        while (infile.read(buffer, sizeof(buffer)) || infile.gcount() > 0) {
            if (send(sock, buffer, infile.gcount(), 0) < 0) {
                std::cerr << "Error sending file\n";
                return false;
            }
        }

        std::cout << "File sent successfully!\n";
        return true;
    }

    void CloseConnection() {
        if (sock != -1) {
            close(sock);
            sock = -1;
        }
    }

    ~FileSender() {
        CloseConnection();
    }
};


int main() {
    FileSender sender("127.0.0.1", 8080);

    if (!sender.ConnectToServer()) return 1;
    if (!sender.SendFile("../data/file_to_send.txt")) return 1;

    return 0;
}
