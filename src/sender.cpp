#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const char* SERVER_IP = "127.0.0.1"; // Server IP
const int PORT = 8080;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported" << std::endl;
        return 1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }

    // Open the file to send
    std::ifstream infile("file_to_send.txt", std::ios::binary);
    if (!infile) {
        std::cerr << "Failed to open file 'file_to_send.txt'" << std::endl;
        close(sock);
        return 1;
    }

    char buffer[1024];
    while (infile.read(buffer, sizeof(buffer)) || infile.gcount() > 0) {
        // Send file contents to the server
        send(sock, buffer, infile.gcount(), 0);
    }

    std::cout << "File sent successfully!" << std::endl;

    // Clean up
    infile.close();
    close(sock);

    return 0;
}
