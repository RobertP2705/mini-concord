#include "sequencer.h"
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

constexpr int PORT = 8080;
constexpr int BUFFER_SIZE = 1024;

Sequencer::Sequencer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        close(server_fd);
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to bind socket");
    }

    if(listen(server_fd, 3) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to listen on socket");
    }

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    int new_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
    if (new_socket < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to accept connection");
    }

    char buffer[BUFFER_SIZE] = {0};
    ssize_t valread = read(new_socket, buffer, BUFFER_SIZE);
    if(valread > 0) {
        std::cout << "Sequencer received: " << buffer << std::endl;
        std::string ack = "ACK from sequencer";
        send(new_socket, ack.c_str(), ack.size(), 0);
    }

    close(new_socket);
    close(server_fd);
}

