#include "trader.h"

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

Trader::Trader() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw std::runtime_error("Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        close(sock);
        throw std::runtime_error("Invalid address/ Address not supported");
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        throw std::runtime_error("Connection Failed");
    }

    std::string hello = "Hello from client";
    send(sock, hello.c_str(), hello.size(), 0);
    std::cout << "Hello message sent" << std::endl;

    ssize_t valread = read(sock, buffer, BUFFER_SIZE);
    if(valread > 0) {
        std::cout << "Received: " << buffer << std::endl;
    }

    close(sock);
}
