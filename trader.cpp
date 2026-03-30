#include "trader.h"

constexpr int PORT = 8080;

void Trader::run(int socket, char buffer[TRADER_BUFFER_SIZE]) {
    while (true) {
        std::string order;
        std::getline(std::cin, order);
        if (order == "exit") {
            break;
        }
        send(socket, order.c_str(), order.size(), 0);
        ssize_t valread = read(socket, buffer, TRADER_BUFFER_SIZE);
        if(valread > 0) {
            std::cout << "Received: " << buffer << std::endl;
        }
    }
}

Trader::Trader() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[TRADER_BUFFER_SIZE] = {0};

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

    run(sock,buffer);
}
